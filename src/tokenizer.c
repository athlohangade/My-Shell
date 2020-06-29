#include "tokenizer.h"

/*  Function to tokenize and parse the 'command' string 
 *  [in param]  : 'command' string
 *  [out param] : 'arguments' contains the tokens of the 'command' string 
 *  [out param] : 'in_file' contains the file name from which input should be taken
 *  [out param] : 'out_file' contains the file name to which result would be outputted
 *  retval      : index value of 'command' which is equal to index value of ending 
                  character (PIPE or EXIT) plus 1 */
int tokenizer_parser(char *command, char *arguments[], char *in_file, char *out_file) {

    /* Local variable */
    int length, i, j, k, arg_type;
    state current_state;
    char temporary_string[128];

    /* Initialize required variables */
    i = 0;
    j = 0;
    k = 0;
    arg_type = 0;
    current_state = INIT;
    length = strlen(command);

    /* Loop for transition from one state to the another of the state diagram according 
       the current state and input character encountered */
    while (i <= length) {

        /* Switch case to take action depending on current state. The inner switch case
           for making transitions based on current character */
        switch (current_state) {

            /* Initial state */
            case INIT :
                    switch (command[i]) {
                        case SPACE  :
                        case TAB    :           current_state = INIT;
                                                break;
                        case IN_REDIRECTION  :
                        case OUT_REDIRECTION :  return -1;
                        case PIPE :
                        case EXIT :             arguments[k] = NULL;
                                                return -1;
                        default :               temporary_string[j++] = command[i];
                                                current_state = ARGS;
                                                arg_type = COMMAND_TYPE;
                   }
                    break;

            /* State when normal characters are encounter (Eg: a-z, A-Z, etc.) */
            case ARGS :
                    switch (command[i]) {
                        case SPACE  :
                        case TAB    :           temporary_string[j] = '\0';
                                                if (arg_type == COMMAND_TYPE) 
                                                    arguments[k++] = strdup(temporary_string);
                                                else if (arg_type == INPUT_TYPE)
                                                    strcpy(in_file, temporary_string);
                                                else if (arg_type == OUTPUT_TYPE)
                                                    strcpy(out_file, temporary_string);
                                                j = 0;
                                                current_state = WHITE_SPACE;
                                                arg_type = COMMAND_TYPE;
                                                break;

                        case IN_REDIRECTION  :  temporary_string[j] = '\0';
                                                if (arg_type == COMMAND_TYPE) 
                                                    arguments[k++] = strdup(temporary_string);
                                                else if (arg_type == INPUT_TYPE)
                                                    strcpy(in_file, temporary_string);
                                                else if (arg_type == OUTPUT_TYPE)
                                                    strcpy(out_file, temporary_string);
                                                j = 0;
                                                arg_type = INPUT_TYPE;
                                                current_state = SPECIAL;
                                                break;

                        case OUT_REDIRECTION :  temporary_string[j] = '\0';
                                                if (arg_type == COMMAND_TYPE) 
                                                    arguments[k++] = strdup(temporary_string);
                                                else if (arg_type == INPUT_TYPE)
                                                    strcpy(in_file, temporary_string);
                                                else if (arg_type == OUTPUT_TYPE)
                                                    strcpy(out_file, temporary_string);
                                                j = 0;
                                                arg_type = OUTPUT_TYPE;
                                                current_state = SPECIAL;
                                                break;

                        case PIPE :
                        case EXIT :             temporary_string[j] = '\0';
                                                if (arg_type == COMMAND_TYPE)
                                                    arguments[k++] = strdup(temporary_string);
                                                else if (arg_type == INPUT_TYPE)
                                                    strcpy(in_file, temporary_string);
                                                else if (arg_type == OUTPUT_TYPE)
                                                    strcpy(out_file, temporary_string); 
                                                j = 0;
                                                arguments[k] = NULL;
                                                if (command[i] == PIPE)
                                                    return i + 1;
                                                else
                                                    return i;
                                                

                        default :               temporary_string[j++] = command[i];
                                                current_state = ARGS;
                    }
                    break;

            /* State when white space is encountered */
            case WHITE_SPACE :
                    switch (command[i]) {
                        case SPACE  :
                        case TAB    :           current_state = WHITE_SPACE;
                                                break;

                        case IN_REDIRECTION  :  j = 0;
                                                arg_type = INPUT_TYPE;
                                                current_state = SPECIAL;
                                                break;

                        case OUT_REDIRECTION :  j = 0;
                                                arg_type = OUTPUT_TYPE;
                                                current_state = SPECIAL;
                                                break;

                        case PIPE :
                        case EXIT :             arguments[k] = NULL;
                                                if (command[i] == PIPE)
                                                    return i + 1;
                                                else
                                                    return i;

                        default :               j = 0;
                                                temporary_string[j++] = command[i];
                                                current_state = ARGS;
                    }
                    break;

            /* State when '<' and '>' are encountered */
            case SPECIAL :
                    switch (command[i]) {
                        case SPACE  :
                        case TAB    :           current_state = WHITE_SPACE;
                                                break;
                        case IN_REDIRECTION :   
                        case OUT_REDIRECTION :  return -1;
                        default :               j = 0;
                                                temporary_string[j++] = command[i];
                                                current_state = ARGS;
                                                break;
                    }
                    break;
            default :
                    break;
        }
        i++;
    }
}