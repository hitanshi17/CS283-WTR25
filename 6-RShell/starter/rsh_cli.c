#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

#define CMD_BUFF_SIZE 1024
#define RSP_BUFF_SIZE 4096

int exec_remote_cmd_loop(char *address, int port)
{
    char *cmd_buff = (char *)malloc(CMD_BUFF_SIZE);
    char *rsp_buff = (char *)malloc(RSP_BUFF_SIZE);
    int cli_socket = -1;
    ssize_t io_size;
    int is_eof;

    if (!cmd_buff || !rsp_buff)
    {
        perror("Memory allocation failed");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_MEMORY);
    }

    cli_socket = start_client(address, port);
    if (cli_socket < 0)
    {
        perror("start_client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1)
    {
        printf("rdsh> ");
        fflush(stdout);

        if (fgets(cmd_buff, CMD_BUFF_SIZE, stdin) == NULL)
        {
            perror("fgets failed");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = 0;

        if (strcmp(cmd_buff, "exit") == 0 || strcmp(cmd_buff, "stop-server") == 0)
        {
            send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0);
            break;
        }

        if (send(cli_socket, cmd_buff, strlen(cmd_buff) + 1, 0) == -1)
        {
            perror("send failed");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        while (1)
        {
            io_size = recv(cli_socket, rsp_buff, RSP_BUFF_SIZE, 0);

            if (io_size < 0)
            {
                perror("recv failed");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            }
            else if (io_size == 0)
            {
                printf("Server disconnected.\n");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
            }

            printf("%.*s", (int)io_size, rsp_buff);

            is_eof = (rsp_buff[io_size - 1] == RDSH_EOF_CHAR) ? 1 : 0;
            if (is_eof)
            {
                break;
            }
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if (cli_socket > 0) {
        close(cli_socket);
    }
    free(cmd_buff);
    free(rsp_buff);
    return rc;
}

int start_client(char *server_ip, int port) {
    struct sockaddr_in addr;
    int cli_socket;

    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket failed");
        return ERR_RDSH_CLIENT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    if (connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}
