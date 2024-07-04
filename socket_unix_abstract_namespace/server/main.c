#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void) {
    int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sun_family = AF_UNIX;

    strncpy(server_addr.sun_path + 1, "server", sizeof server_addr.sun_path - 2);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
        perror("Error when binding");
        return 1;
    };

    while (1) {
        char msg[10] = {0};
        struct sockaddr_un client_addr;
        memset(&client_addr, 0, sizeof client_addr);

        socklen_t client_size;

        int bytes_received = recvfrom(server_fd, msg, sizeof msg, 0, (struct sockaddr *) &client_addr, &client_size);
        if (bytes_received == -1) {
            perror("Error when receiving");
            return 1;
        }

        char *client_path = client_addr.sun_path[0] == 0 ? client_addr.sun_path + 1 : client_addr.sun_path;

        printf("A message of %d bytes received: %.*s, from a client with the path: %.*s\n",
               bytes_received,
               bytes_received, msg,
               (int) sizeof client_addr.sun_path - 1, client_path);

        for (int i = 0; i < bytes_received; i++) {
            msg[i] = toupper(msg[i]);
        }

        if (sendto(server_fd, msg, strnlen(msg, sizeof msg), 0, (struct sockaddr *) &client_addr, client_size) == -1) {
            perror("Error when sending");
            return 1;
        };
    }
}
