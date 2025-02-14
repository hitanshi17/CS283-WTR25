#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

int build_cmd_list(char *cmd_line, command_list_t *cmd_list)
{
    cmd_list->num = 0;

    {
        int spaces = 1; 
        for (int idx = 0; cmd_line[idx] != '\0'; idx++) {
            if (cmd_line[idx] != ' ' && cmd_line[idx] != '\t') {
                spaces = 0;
                break;
            }
        }
        if (spaces) {
            return WARN_NO_CMDS;
        }
    }

    char *pipeMarker;
    char *pipeSegment = strtok_r(cmd_line, "|", &pipeMarker);

    while (pipeSegment != NULL) {
       
        if (cmd_list->num >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        while (*pipeSegment == ' ' || *pipeSegment == '\t') {
            pipeSegment++;
        }

        char *endPtr = pipeSegment + strlen(pipeSegment) - 1;
        while (endPtr >= pipeSegment && (*endPtr == ' ' || *endPtr == '\t')) {
            *endPtr = '\0';
            endPtr--;
        }

        if (*pipeSegment != '\0') {
            char *spaceMarker;
            char *word = strtok_r(pipeSegment, " \t", &spaceMarker);
            if (word) {
                strncpy(cmd_list->commands[cmd_list->num].exe, word, EXE_MAX - 1);
                cmd_list->commands[cmd_list->num].exe[EXE_MAX - 1] = '\0';

                cmd_list->commands[cmd_list->num].args[0] = '\0';

                word = strtok_r(NULL, " \t", &spaceMarker);
                while (word) {
                    if (cmd_list->commands[cmd_list->num].args[0] != '\0') {
                        strncat(cmd_list->commands[cmd_list->num].args, " ",
                                ARG_MAX - 1 - strlen(cmd_list->commands[cmd_list->num].args));
                    }
                    strncat(cmd_list->commands[cmd_list->num].args, word,
                            ARG_MAX - 1 - strlen(cmd_list->commands[cmd_list->num].args));

                    word = strtok_r(NULL, " \t", &spaceMarker);
                }
                cmd_list->num++;
            }
        }

        pipeSegment = strtok_r(NULL, "|", &pipeMarker);
    }

    if (cmd_list->num == 0) {
        return WARN_NO_CMDS;
    }
    return OK;
}
