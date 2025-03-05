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
#ifndef ERR_INVALID
#define ERR_INVALID 2
#endif

static char *in_file = NULL;
static char *out_file = NULL;
static bool append_mode = false;

int last_rc = 0;

int build_cmd_buff(char *line, cmd_buff_t *cb)
{
    cb->_cmd_buffer = malloc(strlen(line) + 1);
    if (!cb->_cmd_buffer) {
        fprintf(stderr, "Memory error\n");
        return ERR_MEMORY;
    }
    strcpy(cb->_cmd_buffer, line);
    char *p = cb->_cmd_buffer;
    while (*p && isspace((unsigned char)*p))
        p++;
    memmove(cb->_cmd_buffer, p, strlen(p) + 1);
    int len = strlen(cb->_cmd_buffer);
    while (len > 0 && isspace((unsigned char)cb->_cmd_buffer[len - 1])) {
        cb->_cmd_buffer[len - 1] = '\0';
        len--;
    }
    cb->argc = 0;
    bool inq = false;
    char *start = NULL;
    p = cb->_cmd_buffer;
    for (int i = 0;; i++) {
        char c = p[i];
        if (c == '\0') {
            if (start)
                cb->argv[cb->argc++] = start;
            break;
        }
        if (c == '"') {
            if (inq) {
                inq = false;
                p[i] = '\0';
                cb->argv[cb->argc++] = start;
                start = NULL;
            } else {
                inq = true;
                start = &p[i + 1];
            }
        } else if (isspace((unsigned char)c) && !inq) {
            if (start) {
                p[i] = '\0';
                cb->argv[cb->argc++] = start;
                start = NULL;
            }
        } else {
            if (!start)
                start = &p[i];
        }
        if (cb->argc >= CMD_ARGV_MAX - 1)
            break;
    }
    cb->argv[cb->argc] = NULL;
    if (inq) {
        fprintf(stderr, "Error: unmatched quotes in command.\n");
        free(cb->_cmd_buffer);
        return ERR_INVALID;
    }
    in_file = NULL;
    out_file = NULL;
    append_mode = false;
    int new_argc = 0;
    for (int i = 0; i < cb->argc; i++) {
        if (strcmp(cb->argv[i], "<") == 0) {
            if (i + 1 < cb->argc) {
                in_file = cb->argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: missing input file after '<'\n");
                return ERR_INVALID;
            }
        } else if (strcmp(cb->argv[i], ">") == 0) {
            if (i + 1 < cb->argc) {
                out_file = cb->argv[i + 1];
                append_mode = false;
                i++;
            } else {
                fprintf(stderr, "Error: missing output file after '>'\n");
                return ERR_INVALID;
            }
        } else if (strcmp(cb->argv[i], ">>") == 0) {
            if (i + 1 < cb->argc) {
                out_file = cb->argv[i + 1];
                append_mode = true;
                i++;
            } else {
                fprintf(stderr, "Error: missing output file after '>>'\n");
                return ERR_INVALID;
            }
        } else {
            cb->argv[new_argc++] = cb->argv[i];
        }
    }
    cb->argv[new_argc] = NULL;
    cb->argc = new_argc;
    return OK;
}

int build_cmd_list(char *line, command_list_t *clist)
{
    if (strlen(line) == 0)
        return WARN_NO_CMDS;
    char *saveptr;
    char *token = strtok_r(line, PIPE_STRING, &saveptr);
    while (token != NULL) {
        if (clist->num >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;
        cmd_buff_t *cb = &clist->commands[clist->num];
        memset(cb, 0, sizeof(cmd_buff_t));
        int rc = build_cmd_buff(token, cb);
        if (rc != OK) {
            for (int i = 0; i < clist->num; i++)
                free(clist->commands[i]._cmd_buffer);
            return rc;
        }
        if (cb->argc > 0)
            clist->num++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    if (clist->num == 0)
        return WARN_NO_CMDS;
    return OK;
}

int single_fork_exec(cmd_buff_t *cb)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        if (in_file != NULL) {
            int fd = open(in_file, O_RDONLY);
            if (fd < 0) {
                perror("open infile");
                exit(errno);
            }
            if (dup2(fd, STDIN_FILENO) < 0) {
                perror("dup2 infile");
                exit(errno);
            }
            close(fd);
        }
        if (out_file != NULL) {
            int fd;
            if (append_mode)
                fd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open outfile");
                exit(errno);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 outfile");
                exit(errno);
            }
            close(fd);
        }
        execvp(cb->argv[0], cb->argv);
        perror("execvp");
        exit(errno);
    } else {
        int status;
        waitpid(pid, &status, 0);
        last_rc = WEXITSTATUS(status);
    }
    return OK;
}

int multi_command_pipeline(command_list_t *clist)
{
    int n = clist->num;
    int pfds[2 * (n - 1)];
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pfds + (2 * i)) < 0) {
            perror("pipe");
            for (int j = 0; j < 2 * i; j++)
                close(pfds[j]);
            return ERR_MEMORY;
        }
    }
    pid_t pids[n];
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            for (int j = 0; j < 2 * (n - 1); j++)
                close(pfds[j]);
            return ERR_EXEC_CMD;
        } else if (pid == 0) {
            if (i > 0) {
                if (dup2(pfds[2 * (i - 1)], STDIN_FILENO) < 0) {
                    perror("dup2 - stdin");
                    exit(errno);
                }
            }
            if (i < n - 1) {
                if (dup2(pfds[2 * i + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 - stdout");
                    exit(errno);
                }
            }
            for (int j = 0; j < 2 * (n - 1); j++)
                close(pfds[j]);
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(errno);
        } else {
            pids[i] = pid;
        }
    }
    for (int j = 0; j < 2 * (n - 1); j++)
        close(pfds[j]);
    int status;
    for (int i = 0; i < n; i++) {
        waitpid(pids[i], &status, 0);
        last_rc = WEXITSTATUS(status);
    }
    return OK;
}

int execute_pipeline(command_list_t *clist)
{
    int n = clist->num;
    if (n < 1)
        return WARN_NO_CMDS;
    if (n == 1) {
        cmd_buff_t *cb = &clist->commands[0];
        if (strcmp(cb->argv[0], "cd") == 0) {
            if (cb->argc > 1) {
                if (chdir(cb->argv[1]) != 0)
                    perror("chdir failed");
            }
            return OK;
        }
        return single_fork_exec(cb);
    }
    return multi_command_pipeline(clist);
}

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
        command_list_t clist;
        memset(&clist, 0, sizeof(clist));
        int rc = build_cmd_list(linebuf, &clist);
        if (rc == WARN_NO_CMDS) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            for (int i = 0; i < clist.num; i++) {
                if (clist.commands[i]._cmd_buffer)
                    free(clist.commands[i]._cmd_buffer);
            }
            continue;
        } else if (rc != OK) {
            for (int i = 0; i < clist.num; i++) {
                if (clist.commands[i]._cmd_buffer)
                    free(clist.commands[i]._cmd_buffer);
            }
            continue;
        }
        if (clist.num == 1 && clist.commands[0].argc > 0 &&
            strcmp(clist.commands[0].argv[0], "exit") == 0) {
            if (clist.commands[0]._cmd_buffer)
                free(clist.commands[0]._cmd_buffer);
            break;
        }
        execute_pipeline(&clist);
        for (int i = 0; i < clist.num; i++) {
            if (clist.commands[i]._cmd_buffer) {
                free(clist.commands[i]._cmd_buffer);
                clist.commands[i]._cmd_buffer = NULL;
            }
        }
    }
    return 0;
}
