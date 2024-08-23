#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>

int fd;

int main() {
    struct termios options;

    fd = open("/dev/ttyS3", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttyS0 - ");
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
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);

    const char *messages[] = {"[VM] OTA Available", "[VM] bbb", "[VM] ccc"};
    for(int i = 0; i < 3; i++) {
        int n = write(fd, messages[i], strlen(messages[i]));
        if (n < 0) {
            perror("Write failed - ");
            return 1;
        }
        tcdrain(fd);
    }
    

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while(1) {
        char buf[256];
        memset(buf, 0, sizeof(buf));

        int poll_result = poll(fds, 1, -1); // Wait indefinitely for data

        if (poll_result > 0) {
            if (fds[0].revents & POLLIN) {
                int n = read(fd, buf, sizeof(buf) - 1);

                if (n < 0) {
                    perror("Read failed -");
                    return -1;
                } else if (n > 0) {
                    printf("Received: %s\n", buf);
                }
            }
        }
    }    

    close(fd);

    return 0;
}