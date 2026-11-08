#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
    int p1[2], p2[2];
    char byte = 'A';
    int pid;

    if (pipe(p1) < 0 || pipe(p2) < 0) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // 子进程
        close(p1[1]);
        close(p2[0]);

        if (read(p1[0], &byte, 1) != 1) {
            fprintf(2, "read failed\n");
            exit(1);
        }

        printf("%d: received ping\n", getpid());

        if (write(p2[1], &byte, 1) != 1) {
            fprintf(2, "write failed\n");
            exit(1);
        }

        close(p1[0]);
        close(p2[1]);
        exit(0);
    } else {
        // 父进程
        close(p1[0]);
        close(p2[1]);

        if (write(p1[1], &byte, 1) != 1) {
            fprintf(2, "write failed\n");
            exit(1);
        }

        if (read(p2[0], &byte, 1) != 1) {
            fprintf(2, "read failed\n");
            exit(1);
        }

        printf("%d: received pong\n", getpid());

        close(p1[1]);
        close(p2[0]);

        wait(0);
        exit(0);
    }
}
