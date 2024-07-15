#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>

#include "thread_pool.h"
#include <unistd.h>

const int EVENT_LIST_SIZE = 100;
const int THREAD_COUNT = 4;
const uint32_t EPOLL_EVENTS = EPOLLIN;

void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        perror("set_nonblock fcntl getfl");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("set_nonblock fcntl nonblock");
        exit(EXIT_FAILURE);
    };
}

void process_client(struct task_arg input) {
    char buf[6] = {0};

    while (1) {
        int size = recv(input.fd, buf, sizeof buf - 1, 0);

        if (size > 0) {
            send(input.fd, buf, size, 0);
            continue;
        }

        // connection closed
        if (size == 0) {
            close(input.fd);
            return;
        }

        // connection reset by peer
        if (errno == ECONNRESET) {
            close(input.fd);
            return;
        }

        // fatal error
        if (errno != EWOULDBLOCK) {
            printf("%d\n", errno);
            perror("recv");
            exit(EXIT_FAILURE);
        }

        // ewouldblock handling
        struct epoll_event event;
        event.data.fd = input.fd;
        event.events = EPOLL_EVENTS;

        if (epoll_ctl(input.epoll_fd, EPOLL_CTL_ADD, input.fd, &event) == -1) {
            perror("epoll_ctl add fd again");
            exit(EXIT_FAILURE);
        };

        return;;
    }
}



int main(void) {
    signal(SIGPIPE, SIG_IGN);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    const int enable = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt SO_REUSEADDR");
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 9000;

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
        perror("bind");
        return 1;
    };

    set_nonblock(listen_fd);
    if (listen(listen_fd, SOMAXCONN) == -1) {
        perror("listen");
        return 1;
    }

    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLL_EVENTS;

    int epoll_fd = epoll_create1(0);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        perror("epoll_ctl add listen_fd");
        return 1;
    };

    struct epoll_event event_list[EVENT_LIST_SIZE];

    struct thread_pool thread_pool;
    tp_init(&thread_pool, THREAD_COUNT);
    tp_start(&thread_pool);

    // entering epoll loop
    while (1) {
        int ready_fds = epoll_wait(epoll_fd, event_list, EVENT_LIST_SIZE, -1);
        if (ready_fds == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("epoll_wait");
            return 1;
        }

        printf("epoll ready %d\n", ready_fds);
        for (int i = 0; i < ready_fds; i++) {
            struct epoll_event e = event_list[i];
            int fd = e.data.fd;

            // accept a client if a server socket event is ready
            if (listen_fd == fd) {
                int connected_fd = accept(listen_fd, NULL, NULL);
                printf("connected fd %d\n", connected_fd);
                set_nonblock(connected_fd);

                event.data.fd = connected_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connected_fd, &event) == -1) {
                    perror("epoll_ctl add connected_fd");
                    return 1;
                };

                continue;
            }

            // // else process a client request
            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
                perror("epoll_ctl del connected_fd");
                return 1;
            };

            struct task_arg arg;
            arg.epoll_fd = epoll_fd;
            arg.fd = fd;

            struct task *task = malloc(sizeof (struct task));
            task->arg = arg;
            task->func = process_client;

            tp_submit(&thread_pool, task);
        }
    }
}
