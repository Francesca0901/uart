#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>

int fd;

void cleanup() {
    if (fd != -1) {
        close(fd);
        printf("Serial port closed\n");
    }
}

void signal_handler(int signo) {
    cleanup();
    exit(0);
}

int main() {
    struct termios options;

    signal(SIGINT, signal_handler);

    fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttyS3 - ");
        return 1;
    }


    tcgetattr(fd, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // disable ICANON, so that we don't need to wait for '\n'
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
    const char *messages[] = {"[SOS] OTA Available", "[SOS] 222", "[SOS] 333"};

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN; 

    while (1) {
        char buf[256];
        memset(buf, 0, sizeof(buf));

        int poll_result = poll(fds, 1, -1); // Wait indefinitely for data

        if (poll_result > 0) {
            if (fds[0].revents & POLLIN) {
                int n = read(fd, buf, sizeof(buf) - 1);
                if (n < 0) {
                    perror("Read failed - ");
                    return 1;
                } else if (n > 0) {
                    printf("Received: %s\n", buf);

                    if (strcmp(buf, "[VM] OTA Available") == 0) {
                        printf("Response: SOS will distribute OTA!\n");
                        n = write(fd, messages[0], strlen(messages[0]));
                        if (n < 0) {
                            perror("Write failed - ");
                            return 1;
                        }
                    } else if (strcmp(buf, "[VM] bbb") == 0) {
                        printf("Response: handle bbb\n");
                        n = write(fd, messages[1], strlen(messages[1]));
                    } else if (strcmp(buf, "[VM] ccc") == 0) {
                        printf("Response: handle ccc\n");
                        n = write(fd, messages[2], strlen(messages[2]));
                    } else {
                        printf("Response: unhandle message.\n");
                    }
                }
            }
        } else if (poll_result == 0) {
            printf("Poll timed out\n");
        } else {
            perror("Poll error");
            break;
        }
    }

    cleanup();

    return 0;
}