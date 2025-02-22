#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h> 

#ifndef CMD_ERR_EXECUTE
#define CMD_ERR_EXECUTE "Command failed to execute"
#endif


/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    cmd_buff-> _cmd_buffer = malloc(strlen(cmd_line) + 1);
    if (!cmd_buff->_cmd_buffer) {
        fprintf(stderr, "Memory error\n");
        return ERR_MEMORY;
    }
    strcpy(cmd_buff->_cmd_buffer, cmd_line);

    char *buf = cmd_buff->_cmd_buffer;
    while (*buf && isspace((unsigned char)*buf)) {
        buf++;
    }
    memmove(cmd_buff->_cmd_buffer, buf, strlen(buf) + 1);

    int len = strlen(cmd_buff->_cmd_buffer);
    while (len > 0 && isspace((unsigned char)cmd_buff->_cmd_buffer[len - 1])) {
        cmd_buff->_cmd_buffer[len - 1] = '\0';
        len--;
    }

    cmd_buff->argc = 0;
    bool in_quotes = false;
    char *arg_start = NULL;
    char *p = cmd_buff->_cmd_buffer;

    for (int i = 0; ; i++) {
        char c = p[i];
        if (c == '\0') {
            if (arg_start) {
                cmd_buff->argv[cmd_buff->argc++] = arg_start;
            }
            break;
        }
        if (c == '"') {
            if (in_quotes) {
                in_quotes = false;
                p[i] = '\0';
                cmd_buff->argv[cmd_buff->argc++] = arg_start;
                arg_start = NULL;
            } else {
                in_quotes = true;
                arg_start = &p[i + 1];
            }
        } else if (isspace((unsigned char)c) && !in_quotes) {
            if (arg_start) {
                p[i] = '\0';
                cmd_buff->argv[cmd_buff->argc++] = arg_start;
                arg_start = NULL;
            }
        } else {
            if (!arg_start) {
                arg_start = &p[i];
            }
        }
        if (cmd_buff->argc >= CMD_ARGV_MAX - 1)
            break;
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}


int last_rc = 0;

int exec_local_cmd_loop()
{
    while (1) {
        printf("%s", SH_PROMPT);
        char linebuf[SH_CMD_MAX];
        if (fgets(linebuf, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        linebuf[strcspn(linebuf, "\n")] = '\0';
        if (strlen(linebuf) == 0)
            continue;

        cmd_buff_t cmd;
        memset(&cmd, 0, sizeof(cmd));
        if (build_cmd_buff(linebuf, &cmd) != OK) {
            continue;
        }

        if (cmd.argc == 0) {
            free(cmd._cmd_buffer);
            continue;
        }

        if (strcmp(cmd.argv[0], "exit") == 0) {
            free(cmd._cmd_buffer);
            break;
        }
        else if (strcmp(cmd.argv[0], "cd") == 0) {
            if (cmd.argc > 1) {
                if (chdir(cmd.argv[1]) != 0)
                    perror("chdir failed");
            }
        }
        else if (strcmp(cmd.argv[0], "rc") == 0) {
            printf("%d\n", last_rc);
        }
        else {
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "%s\n", CMD_ERR_EXECUTE);
            }
            else if (pid == 0) {
                execvp(cmd.argv[0], cmd.argv);
                int err = errno;
                if (err == ENOENT) {
                    fprintf(stderr, "Command not found in PATH\n");
                } else if (err == EACCES) {
                    fprintf(stderr, "Permission denied\n");
                } else {
                    fprintf(stderr, "%s\n", CMD_ERR_EXECUTE);
                }
                exit(err);  
            }
            else {
                int status;
                waitpid(pid, &status, 0);
                last_rc = WEXITSTATUS(status);
            }
        }
        free(cmd._cmd_buffer);
    }
    return 0;
}
