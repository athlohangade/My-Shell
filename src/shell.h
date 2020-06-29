/* General definitions */
#define COMMAND_LENGTH      256
#define FILENAME_LENGTH     128
#define NO_OF_ARGUMENTS     64
#define MAX_NO_OF_PIPES     8
#define MAX_BG_PROCESS      8
#define PATH_MAX            1024

typedef struct jobs_table {
    pid_t pgid[MAX_BG_PROCESS];
    char *job_name[MAX_BG_PROCESS];
    int pgid_i;
} jobs_table;

void initialize_pids();
void sigint_handler();
void sigchld_handler();
void fg_command_handler(char *arg);
void bg_command_handler(char *arg);
