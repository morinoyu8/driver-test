#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "testdriver.h"

int main(void) {
    
    int fd;
    struct testdevice_values values_set;
    struct testdevice_values values_get;
    values_set.val1 = 1;
    values_set.val2 = 2;

    if ((fd = open("/dev/led0", O_RDWR)) < 0) 
        perror("open");

    if (ioctl(fd, TESTDEVICE_SET_VALUES, &values_set) < 0) 
        perror("ioctl_set");

    if (ioctl(fd, TESTDEVICE_GET_VALUES, &values_get) < 0) 
        perror("ioctl_set");

    printf("val1 = %d, val2 = %d\n", values_get.val1, values_get.val2);

    int input;
    while (1) {
        scanf("%d", &input);
        if (input == 0 || input == 1) {
            input += '0';
            if (write(fd, &input, sizeof(input)) < 0) {
                perror("write");
                break;
            }
        } else {
            break;
        }
    }

    if (write(fd, "0", sizeof(input)) < 0)
        perror("write");

    if (close(fd) != 0) 
        perror("close");

    return 0;
}