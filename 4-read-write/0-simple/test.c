#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    char buff[256];
    int fd0_A, fd0_B, fd1_A;

    if ((fd0_A = open("/dev/testdevice0", O_RDWR)) < 0) 
        perror("open");
    if ((fd0_B = open("/dev/testdevice0", O_RDWR)) < 0) 
        perror("open");
    if ((fd1_A = open("/dev/testdevice1", O_RDWR)) < 0) 
        perror("open");

    printf("Correct:\n");

    if (write(fd0_A, "0_A", 4) < 0) 
        perror("write");
    if (read(fd0_A, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);

    if (write(fd0_B, "0_B", 4) < 0) 
        perror("write");
    if (read(fd0_B, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);

    if (write(fd1_A, "1_A", 4) < 0) 
        perror("write");
    if (read(fd1_A, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);

    printf("Incorrect:\n");

    if (write(fd0_A, "0_A", 4) < 0) 
        perror("write");
    if (write(fd0_B, "0_B", 4) < 0) 
        perror("write");
    if (write(fd1_A, "1_A", 4) < 0) 
        perror("write");

    if (read(fd0_A, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);
    if (read(fd0_B, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);
    if (read(fd1_A, buff, 4) < 0) 
        perror("read");
    printf("%s\n", buff);

    if (close(fd0_A) != 0) 
        perror("close");
    if (close(fd0_B) != 0) 
        perror("close");
    if (close(fd1_A) != 0) 
        perror("close");

    return 0;
}