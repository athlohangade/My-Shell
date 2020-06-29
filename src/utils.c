#include "utils.h"

/*  Function to count number of pipes.
 *  [in param] : 'command' string
 *  retval     : number of pipes */   
int count_pipes(char *command) {
    int i, count;
    i = 0;
    count = 0;

    while (command[i]) {
        if (command[i] == '|') {
            count++;
        }
        i++;
    }
    return count;
}

/* Shell Decoration */
void shell_init() {
    printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("----------------------------------------------------------------------\n");
    printf("************************** WELCOME TO THE SHELL **********************\n");
    printf("----------------------------------------------------------------------\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
    printf("** Rules to be followed :\n");
    printf(" * Type 'exit' to EXIT the shell\n");
    printf(" * Type 'jobs' command to give PIDs of the processes that are running in background or are suspended\n");
    printf(" * Press 'Ctrl-D' to stop giving input to the shell\n");
    printf(" * Press 'Ctrl-C' to end the process\n");
    printf(" * Press 'Ctrl-Z' to suspend the process\n");
    printf(" * To run the process in background, type the command followed by '&'\n");
    printf(" * Give space before '&' to run the process in background (limitation)\n");
    printf(" * For sending the process in background, type 'bg pid_of_the_process'\n");
    printf(" * To bring the process in foreground, type 'fg pid_of_the_process'\n");
    printf(" * PID of the process can be found out using command 'ps' or 'jobs' \n");
    printf(" * Write the commands 'exit' and 'jobs' without any spaces or any additional arguments\n\n");
}
