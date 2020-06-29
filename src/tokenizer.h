#include <string.h>

/* Allowed character definitions */
#define OUT_REDIRECTION     '>'
#define IN_REDIRECTION      '<'
#define SPACE               ' '
#define TAB                 '\t'
#define PIPE                '|' 
#define EXIT                '\0'
 
 /* Flag definitions for local use */
#define COMMAND_TYPE        3
#define INPUT_TYPE          4
#define OUTPUT_TYPE         5

/* States in the state diagram for tokenizer */
typedef enum state {
    INIT,
    ARGS, 
    WHITE_SPACE,
    SPECIAL,
} state;

int tokenizer_parser(char *command, char *arguments[], char *in_file, char *out_file);