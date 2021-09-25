#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

int main()
{
    int listen_fd, max_fd;
    int conn_fd;
    int *client_fds;
    int client_arr_cap, client_arr_size;
    int i;
    char buf[1024];
    fd_set allfds, readfds;
    struct timeval timeout;
    struct sockaddr_in server_addr;

    FD_ZERO(&allfds);
    max_fd = 0;
    client_arr_cap = 5;
    client_arr_size = 0;
    client_fds = (int *) malloc(sizeof(int) * client_arr_cap);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        printf("Create socket failed\n");
        goto fail;
    }
    memset(&server_addr, 0, sizeof(struct sockaddr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6666);
    if (!inet_aton("0.0.0.0", &(server_addr.sin_addr))) {
        printf("Invalid bind address\n");
        goto fail;
    }
    if (bind(listen_fd, (const struct sockaddr*)&server_addr,
             (socklen_t) sizeof(server_addr)) == -1) {
        printf("Server socket binding fail\n");
        goto fail;
    }

    printf("Server listen at 0.0.0.0:6666\n");
    FD_SET(listen_fd, &allfds);
    max_fd = listen_fd;

    do {
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000;
        readfds = allfds;
        if (select(max_fd+1, &readfds, NULL, NULL, &timeout) > 0) {
            if (FD_ISSET(listen_fd, &readfds)) {
                if (listen(listen_fd, 10) == 0) {
                    conn_fd = accept(listen_fd, NULL, NULL);
                    printf("Connection fd %d\n", conn_fd);
                    if (client_arr_size == client_arr_cap) {
                        int *tmp = (int *) malloc(sizeof(int) * client_arr_cap * 2);
                        memcpy(tmp, client_fds, sizeof(int) * client_arr_size);
                        free(client_fds);
                        client_fds = tmp;
                        client_arr_cap *= 2;
                    }
                    client_fds[client_arr_size++] = conn_fd;
                    max_fd = max_fd >= conn_fd ? max_fd : conn_fd;
                    FD_SET(conn_fd, &allfds);
                }
            } else {
                for (i = 0; i < client_arr_size; i++) {
                    if (FD_ISSET(client_fds[i], &readfds)) {
                        read(client_fds[i], buf, sizeof(buf));
                        printf("Read msg from fd %d: %s\n", client_fds[i], buf);
                    }
                }
            }
        }
    } while (1);
    printf("Hello world!\n");
    return 0;

fail:
    return 1;
}
