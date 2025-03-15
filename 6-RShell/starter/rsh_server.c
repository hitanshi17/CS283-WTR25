#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"

int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;
    struct sockaddr_in addr;
    
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) return svr_socket;

    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}

int stop_server(int svr_socket) {
    return close(svr_socket);
}

int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;

    while (1) {
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        rc = exec_client_requests(cli_socket);

        if (rc == OK_EXIT) {
            break;
        }
    }

    stop_server(svr_socket);
    return rc;
}

int exec_client_requests(int cli_socket) {
    ssize_t io_size;
    char *io_buff;
    command_list_t cmd_list;
    int rc;
    
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        return ERR_RDSH_SERVER;
    }

    while (1) {
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (io_size < 0) {
            perror("recv");
            free(io_buff);
            return ERR_RDSH_COMMUNICATION;
        } else if (io_size == 0) {
            printf("Client disconnected.\n");
            free(io_buff);
            return OK;
        }

        io_buff[io_size] = '\0';  

        if (strcmp(io_buff, "exit") == 0) {
            free(io_buff);
            return OK;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            free(io_buff);
            return OK_EXIT;
        }

        rc = build_cmd_list(io_buff, &cmd_list);
        if (rc != OK) {
            send_message_string(cli_socket, "Error parsing command\n");
            send_message_eof(cli_socket);
            continue;
        }

        rc = execute_pipeline(&cmd_list);

        send_message_eof(cli_socket);
    }

    free(io_buff);
    return OK;
}

int send_message_string(int cli_socket, char *buff) {
    if (send(cli_socket, buff, strlen(buff), 0) < 0) {
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

int send_message_eof(int cli_socket) {
    char eof = RDSH_EOF_CHAR;
    if (send(cli_socket, &eof, sizeof(eof), 0) < 0) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}
