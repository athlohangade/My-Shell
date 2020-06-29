#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <pwd.h>
#include "tokenizer.h"
#include "utils.h"
#include "shell.h"

/* Global variable for pgids */
jobs_table all_pgids;

/* Initialize all pids to zero */
void initialize_pids() {

    /* Iterator */
    int i = 0;
    /* Make all the pgids zero and the name of process as NULL */
    while (i < MAX_BG_PROCESS) {
        all_pgids.pgid[i] = 0;
        all_pgids.job_name[i] = NULL;
        i++;
    }
    all_pgids.pgid_i = 0;
}

/* Handler to be executed if SIGINT signal is arrived */
void sigint_handler() {

    /* Print prompt */
    printf("\n");
    print_prompt();
    fflush(stdout);
}

/* Handler to be executed if SIGCHLD signal is arrived */
void sigchld_handler() {

    /* To avoid zombie process */
    waitpid(-1, NULL, WNOHANG);
}

/* Handler for fg command
 * [in param] : 'arg' string containing PID */
void fg_command_handler(char *arg) {

    /* Ignore SIGTTOU */ 
    signal(SIGTTOU, SIG_IGN);

    /* Local variables */
    pid_t pid, temp;
    int i = 0, status;

    /* Convert string to integer */
    pid = atoi(arg);
    while (all_pgids.pgid[i] != pid) {
        i++;
    }
    /* Get the pid in temp variable */
    temp = all_pgids.pgid[i];

    /* Stop the process */
    kill(-1 * all_pgids.pgid[i], SIGTSTP);

    /* Bring the process in foreground */
    tcsetpgrp(STDIN_FILENO, all_pgids.pgid[i]);

    /* Continue the process */
    kill(-1 * all_pgids.pgid[i], SIGCONT);

    /* Wait for the process to complete */
    waitpid(-1 * temp, &status, WUNTRACED);

    /* If process was exited successfully or killed, then remove its entry
     * from jobs table */
    if ((WIFEXITED(status)) || (WIFSIGNALED(status))) {
        /* Decrement PGID count */
        all_pgids.pgid_i--;
        /* Free the memory allocated for proces name */
        free(all_pgids.job_name[i]);
        /* Shift the remaining entries */
        while (i < all_pgids.pgid_i) {
            all_pgids.job_name[i] = all_pgids.job_name[i+1];
            all_pgids.pgid[i] = all_pgids.pgid[i+1];
            i++;
        }
    }

    /* Bring the shell in foreground */
    tcsetpgrp(STDIN_FILENO, getpid());
    /* Set SIGTTOU back to default */
    signal(SIGTTOU, SIG_DFL);
}

/* Handler for bg command
 * [in param] : 'arg' string containing PID */
void bg_command_handler(char *arg) {

    /* Local variables */
    pid_t pid;
    int i = 0;

    /* Convert string to integer */
    pid = atoi(arg);
    while (all_pgids.pgid[i] != pid) {
        i++;
    }

    /* Send the CONTINUE signal to the suspended process to resume */
    kill(-1 * all_pgids.pgid[i], SIGCONT);
}

/* Prints a pretty prompt with current working directory */                                                                             
void print_prompt() {                                                            
    char cwd[PATH_MAX];                                                         
    if(getcwd(cwd, sizeof(cwd)) == NULL) {                                      
        perror("getcwd");                                                       
        exit(EXIT_FAILURE);                                                     
    }
    printf("[ %s ] > ", cwd);
}

/* Main function */
int main() {

    /* Local variables */
    char command[COMMAND_LENGTH];
    char *arguments[NO_OF_ARGUMENTS];
    char input[FILENAME_LENGTH] = "\0";
    char output[FILENAME_LENGTH] = "\0";
    int no_of_pipes, iterator, position_of_next_command, pid, temp, temp2, background_flag, status, fg_flag;
    int fd_read, fd_write;
    int pfd[MAX_NO_OF_PIPES][2];
    pid_t first_child_pid;

    /* Initialize required variables */
    temp = 0;
    iterator = 0;
    first_child_pid = 0;
    position_of_next_command = 0;
    background_flag = 0;
    fg_flag = 0;
    status = 0;
    fd_read = 0; 
    fd_write = 0;

    /* Print shell initials*/
    shell_init();
    /* Initialize global 'pid' variable */
    initialize_pids();

    /* Shell Loop */
    while(1) {

        /* Set the required handler for the signals */ 
        signal(SIGINT, sigint_handler);
        signal(SIGCHLD, sigchld_handler);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        /* Print the prompt */
        print_prompt();
        //printf("prompt > ");

        /* Take the input */
        gets(command);

        /* If just Enter was pressed */
        if (command[0] == '\0') {
            continue;
        }

        /* Check if string equals "exit". If so, exit the shell */
        if (!(strcmp(command , "exit"))) {
            break;
        }

        /* */
        if(strncmp(command, "cd ", 3) == 0) {                              
            char *token = strtok(command, " ");                                 
            token = strtok(NULL, " ");                                          
            if(chdir(token) == -1) {                                            
                perror("cd");                                                   
            }                                                                   
            continue;                                                           
        }

        /* If 'jobs' command is typed */
        if (!(strcmp(command , "jobs"))) {
            temp = 0;
            printf("-----------------------------------\n");
            printf("\tPID\tNAME\n");
            printf("-----------------------------------\n");

            /* Print the jobs-table */
            while (temp < all_pgids.pgid_i) {
                printf("\t%d\t%s\n", all_pgids.pgid[temp], all_pgids.job_name[temp]);
                temp++;
            }
            printf("-----------------------------------\n");
            temp = 0;
            continue;
        }

        /* Count the number of pipes in 'command' */
        no_of_pipes = count_pipes(command);

        /* Loop to execute each subcommand in 'command' string which are separated by '|' */
        while(iterator <= no_of_pipes) {

            /* Get the tokens (command and arguments), input file, output file */
            temp = tokenizer_parser(&command[position_of_next_command], arguments, input, output);
            /* Condition for invalid command */
            if (temp == -1) {
                break;
            }

            /* Find the position of next subcommand using current command position and 
               and length of current command (temp) */
            position_of_next_command = position_of_next_command + temp;
            
            /* If 'fg' command is typed [USAGE : fg PID] */
            if (!(strcmp(arguments[0], "fg"))) {

                /* Check if PID is given */
                if (arguments[1]) {

                    /* Convert the PID string into integer */
                    temp2 = atoi(arguments[1]);
                    temp = 0;

                    /* Find whether the PID is present in the job table */
                    while (temp < all_pgids.pgid_i) {
                        if (temp2 == all_pgids.pgid[temp])
                            break;
                        temp++;
                    }
                    /* If the PID is present in job-table */
                    if (temp != all_pgids.pgid_i) {

                        /* Call the handler and then set the flag */
                        fg_command_handler(arguments[1]);
                        fg_flag = 1;
                    }
                    /* PID not found in job-table */
                    else {
                        printf("Error : Enter a valid PID\n");
                    }
                }
                /* PID not given */
                else {
                    printf("Usage : fg 'PID'\n");
                }
                break;
            }

            /* If 'bg' command is typed [USAGE : bg PID] */
            if (!(strcmp(arguments[0], "bg"))) {

                /* Check if PID is given */
                if (arguments[1]) {

                    /* Convert the PID string into integer */
                    temp2 = atoi(arguments[1]);
                    temp = 0;

                    /* Find whether the PID is present in the job table */
                    while (temp < all_pgids.pgid_i) {
                        if (temp2 == all_pgids.pgid[temp])
                            break;
                        temp++;
                    }

                    /* If the PID is present in job-table */
                    if (temp != all_pgids.pgid_i) {
                        bg_command_handler(arguments[1]);
                    }
                    /* PID not found in job-table */
                    else {
                        printf("Error : Enter a valid PID\n");
                    }
                }
                /* PID not given */
                else {
                    printf("Usage : fg 'PID'\n");
                }
                break;
            }

            /* Check if the command contain '&'. */	
            temp = 0;
            while (arguments[temp]) {
            /* If '&' is found, set the background flag and break */
                if (!(strcmp(arguments[temp], "&"))) {
                    arguments[temp] = NULL;
                    background_flag = 1;
                    break;
                }
                temp++;
            }
            temp = 0;

            /* Create the pipe */
            pipe(pfd[iterator]);

            /* Fork i.e. create the child process */
            pid = fork();

            /* Code to be executed in child process */
            if (pid == 0) {

                /* Set the signal to default */
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                signal(SIGCHLD, SIG_DFL);

                /* Get pid of current process */
                pid = getpid();
                /* For process separated by pipes, the first process PID is set as process 
                 * group id and for further process make its group id same as that of first process */
                if (iterator == 0)
                    first_child_pid = pid;
                setpgid(pid, first_child_pid);

                /* If input file is given as input, redirect the STDIN to that file */
                if (input[0] != '\0') {
                    fd_read = open(input, O_RDONLY);
                    close(0);
                    dup(fd_read);
                    close(fd_read);
                }
                /* All commands except first takes the output of previous command as input
                 * from the pipe.
                 * First command takes input from STDIN or input file if specified. */
                else {
                    if (iterator != 0) {
                        close(0);
                        dup(pfd[iterator - 1][0]);
                        close(pfd[iterator - 1][0]);
                    }
                }

                /* If output file is given as input, redirect the STDOUT to that file */
                if (output[0] != '\0') {
                    fd_write = open(output, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                    close(1);
                    dup(fd_write);
                    close(fd_write);
                }
                /* All commands except last outputs the result of current command in the pipe
                 * Last command outputs the result to STDOUT or output file if specified. */
                else {
                    if (iterator != no_of_pipes) {
                        close(1);
                        dup(pfd[iterator][1]);
                        close(pfd[iterator][1]);
                    }
                }

                /* Execute the command and check the retval if it is executed successfully */
                if (execvp(arguments[0], arguments) == -1) {
                    printf("Invalid Command !!!\n");
                    exit(0);
                }
            }
            /* Code to be executed in parent process */
            else {

                /* Same as done above */
                if (iterator == 0)
                    first_child_pid = pid;
                setpgid(pid, first_child_pid);

                /* Close the write end of the pipe to let the child read from it */
                close(pfd[iterator][1]);
            }

            /* Increment the iterator */
            iterator++;

            /* Clear the input and output filename */
            input[0] = '\0';
            output[0] = '\0';
            temp = 0;

            /* Free the memory allocated for arguments of command */
            while (arguments[temp]) {
                free(arguments[temp++]);
            }
        }
        /* Check the foreground flag */
        if (fg_flag == 0) {
            if (!background_flag) {

                /* Set the process group to foreground */
                tcsetpgrp(STDIN_FILENO, first_child_pid);

                temp = 0;
                /* Wait for the number of child process */
                while (temp <= no_of_pipes) {

                    /* Wait for child and get status of child process */
                    waitpid(-1 * first_child_pid, &status, WUNTRACED);

                    /* If Ctrl-Z was pressed to suspend the process */
                    if (WIFSTOPPED(status)) {
                        iterator = 0;

                        /* Check if the PGID is already present in the job-table */
                        while (iterator < all_pgids.pgid_i) {
                            if (first_child_pid == all_pgids.pgid[iterator])
                                break;
                            iterator++;
                        }

                        /* If PGID is not present, add it to job-table */
                        if (iterator == all_pgids.pgid_i) {
                            all_pgids.job_name[all_pgids.pgid_i] = strdup(command);
                            all_pgids.pgid[all_pgids.pgid_i++] = first_child_pid;
                        }
                    }
                    temp++;
                }

                /* Get PID of the shell */
                temp = getpid();
                /* Ignore the SIGTTOU signal to allow the shell to come to foreground */
                signal(SIGTTOU, SIG_IGN);
                /* Set the shell to foreground */
                tcsetpgrp(STDIN_FILENO, temp); 
            }
            /* If the process was started in background (using '&'), just add entry 
             * in job-table and don't wait for child process */
            else {
                all_pgids.job_name[all_pgids.pgid_i] = strdup(command);
                all_pgids.pgid[all_pgids.pgid_i++] = first_child_pid;
            }
        }

        /* Set the variables to default state for next command */
        temp = 0;
        iterator = 0;
        first_child_pid = 0;
        position_of_next_command = 0;
        background_flag = 0;
        fg_flag = 0;
        status = 0;
        fd_read = 0; 
        fd_write = 0;
    }
    /* Free the jobs pointers */
    temp = 0;
    while (temp < all_pgids.pgid_i) {
        free(all_pgids.job_name[temp]);
        all_pgids.pgid[temp] = 0;
        temp++;
    }
    return 0;
}
