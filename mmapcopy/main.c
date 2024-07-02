#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void print_error_msg(char* error_msg);
ssize_t reliable_write(int fd, char *buf, size_t n);

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        print_error_msg("Error: You should specify the file to output\n");
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        print_error_msg("Error: You should specify one file, which has a name without any spaces\n");
        return EXIT_FAILURE;
    }

    char *file_name = argv[1];

    int file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor == -1) {
        perror("Error");
        return EXIT_FAILURE;
    }

    struct stat file_info;
    fstat(file_descriptor, &file_info);

    size_t file_size = file_info.st_size;

    void* file_content = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
    if (file_content == MAP_FAILED) {
        perror("Error");
        return EXIT_FAILURE;
    }

    if (reliable_write(STDOUT_FILENO, file_content, file_size) == -1) {
        perror("Error");
        return EXIT_FAILURE;
    };

    return EXIT_SUCCESS;
}

ssize_t reliable_write(int fd, char *buf, size_t n) {
    size_t left = n;

    while (left > 0) {
        ssize_t written = write(fd, buf, left);

        if (written <= 0) {
            if (errno != EINTR) {
                return -1;
            }
            written = 0;
        }

        left -= written;
        buf += written;
    }

    return n;
}

void print_error_msg(char* error_msg) {
    reliable_write(STDERR_FILENO, error_msg, strlen(error_msg));
}

