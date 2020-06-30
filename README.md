# My-Shell
This project is an implementation of Unix shell, a simple command interpreter.

## How to run
-   Change working directory to *src* :

-   Enter following command :
    >   make

-   Run the executable :
    >   ./shell

## Features

- Prompt having current working directory

- Allows the user to execute one or more programs from executable files on the file-system, as background or foreground jobs

- Allows for the piping ( | ) of several tasks as well as input ( < ) and output ( > ) redirection

- Provides job-control, including a job list and control for changing the foreground/background status of currently running jobs and job continuation/termination

- Signals like Ctrl-C and Ctrl-Z

- Built in commands like `cd`, `exit`, etc

## Built-in commands

- `cd` - Change directory using `chdir(2)`

- `jobs` - Prints out the command line strings for jobs that are currently executing in the background or are currently suspended, as well as the identifier (process id) associated with each command line string by maintaining a jobs table.

- `fg` - Brings the process with given pid to the foreground using `tcsetpgrp(3)`

- `bg` - Runs the stopped process with given pid in background, relinquishing shell control yet still logging to shell

- `exit` - Exits the shell

## Usage

-   The conventions for using the shell are same as that of *bash*. *My-Shell* follows the same rules as that of *bash*.

-   There are some exceptions in case of `fg`, `bg` commands. In *bash*, these commands take the most recent process and resume its execution in foreground or background. *My-Shell* follows different input format as `fg/bg <pid_of_the_process>` to take the specific process and resume its execution.

## Implementation
-   The shell prints a prompt and waits for a command line.

-   The command line consists of one or more commands and 0 or more arguments for every command separated by one or more spaces and pipes (|). The last command is optionally followed by an ampersand &.

-   The command line is then parsed and broken down into information like arguments, input/output files or background process/not using a state machine.

-   The shell then creates a child process (using `fork(2)`) to load and execute the program (using `execvp(2)`) for *command*.

-   If command's input/output  is redirected/piped, appropriate opening and closing of file descriptors is done using `dup(2)`.

-   The first child process forked for execution is made the group leader of a new process group. All other process except the first process for the same command set their group id equal to process of the first process using `setpgid(2)`.

-   The shell waits for commands it executes as foreground processes, but not for those executed as background processes (using `waitpid(2)` and custom `SIGCHLD` handler).

-   Ctrl-Z generates a `SIGTSTP`. This suspends the processes in the current foreground job using `kill(2)`. If there is no foreground job, it has no effects.

-   Ctrl-C generates a `SIGINT`. This causes the shell to terminate the processes in the current foreground job using `kill(2)`. If there is no foreground job, it has no effects.

-   When a process is sent to background or suspended, an entry is noted in jobs table which contains process name and its identifier (process_id). This information is useful for resuming the suspended process or bringing to foreground

## Authors
[Atharva Lohangade](https://github.com/athlohangade)