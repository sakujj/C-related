#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void) {
    int client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof client_addr);
    client_addr.sun_family = AF_UNIX;

    strncpy(client_addr.sun_path + 1, "client", sizeof client_addr.sun_path - 2);

    if (bind(client_fd, (struct sockaddr *) &client_addr, sizeof client_addr) == -1) {
        perror("Error when binding");
        return 1;
    };

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sun_family = AF_UNIX;

    strncpy(server_addr.sun_path + 1, "server", sizeof server_addr.sun_path - 2);

    char msg[11];

    if (fgets(msg, sizeof msg, stdin) == NULL) {
        if (ferror(stdin) != 0) {
            return 1;
        }

        strcpy(msg, "eof");
    };


    int bytes_sent = sendto(client_fd, msg, strnlen(msg, sizeof msg), 0, (struct sockaddr *) &server_addr,
                            sizeof server_addr);
    if (bytes_sent == -1) {
        perror("Error when sending");
        return 1;
    }

    printf("Send %d bytes message: %s\n", bytes_sent, msg);

    int bytes_received = recvfrom(client_fd, msg, sizeof msg, 0, NULL, NULL);
    if (bytes_received == -1) {
        perror("Error when receiving");
        return 1;
    }

    printf("A message of %d bytes received: %.*s\n", bytes_received, bytes_received, msg);

    return 0;
}
