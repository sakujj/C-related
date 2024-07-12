#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <time.h>

const size_t NUMBER_OF_CLIENTS = 5;

const int MSG_LENGTH = 30;

const long SEC_BEFORE_NEXT_COMMUNCATION = 1;
const long NANO_SEC_BEFORE_NEXT_COMMUNCATION = 0;

int main(void) {
    int client_fds[NUMBER_OF_CLIENTS];
    for (int i = 0; i < NUMBER_OF_CLIENTS; ++i) {
        client_fds[i] = socket(AF_INET, SOCK_STREAM, 0);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 9000;


    for (int i = 0; i < NUMBER_OF_CLIENTS; ++i) {
        if (connect(client_fds[i], (struct sockaddr*) &server_addr, sizeof (struct sockaddr_in)) == -1) {
            perror("connect");
            fprintf(stderr, "connect client_fd %d", client_fds[i]);
            return -1;
        };
    }

    char client_msgs[NUMBER_OF_CLIENTS][MSG_LENGTH];

    while(1) {

        for (int i = 0; i < NUMBER_OF_CLIENTS; ++i) {
            char symbol[2];
            snprintf(symbol,2, "%d", i % 10);
            for (int j = 0; j < MSG_LENGTH; j++) {
                client_msgs[i][j] = symbol[0];
            }
        }


        for (int i = 0; i < NUMBER_OF_CLIENTS; ++i) {
            int bytes_sent = send(client_fds[i], client_msgs[i], MSG_LENGTH, 0);
            if (bytes_sent == -1) {
                fprintf(stderr, "Error when sending with fd %d", client_fds[i]);
                return 1;
            }
            printf("Send %d bytes message: %.*s\n", bytes_sent, MSG_LENGTH, client_msgs[i]);
        }

        for (int i = 0; i < NUMBER_OF_CLIENTS; ++i) {
            int bytes_received = recv(client_fds[i], client_msgs[i], sizeof client_msgs[i], 0);
            if (bytes_received == -1) {
                fprintf(stderr, "Error when receiving with fd %d", client_fds[i]);
                return 1;
            }
            printf("A message of %d bytes received: %.*s\n", bytes_received, bytes_received, client_msgs[i]);
        }

        // for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
        //     close(client_fds[i]);
        // }
        // break;

        fflush(stdout);
        struct timespec timespec;
        timespec.tv_sec = SEC_BEFORE_NEXT_COMMUNCATION;
        timespec.tv_nsec = NANO_SEC_BEFORE_NEXT_COMMUNCATION;
        nanosleep(&timespec, NULL);

    }
    return 0;
}
