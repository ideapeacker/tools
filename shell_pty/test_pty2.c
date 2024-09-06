#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <utmp.h>
#include <pty.h>
#include <string.h>
int main()
{

    int master_fd, slave_fd;
    char *slave_name;
    char command[] = "/bin/bash";

    // 创建新的pty
    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) == -1)
    {
        perror("openpty");
        exit(1);
    }

    // 通过slave名称获取pts设备
    // slave_name = ptsname(master_fd);
    slave_name = ttyname(master_fd);
    // 创建并执行bash
    if (fork() == 0)
    {
        // 关闭不需要的文件描述符
        close(master_fd);

        // 将slave设备重定向到标准输入输出，并执行命令
        if (login_tty(slave_fd) == -1)
        {
            perror("login_tty");
            exit(1);
        }
        // 将slave端设置为当前的标准输入、输出、错误输出
        dup2(slave_fd, 0);
        dup2(slave_fd, 1);
        dup2(slave_fd, 2);
        if (execl(command, command, (char *)NULL) == -1)
        {
            perror("execl");
            exit(1);
        }

        exit(0);
    }

    // 父进程中，关闭slave端
    close(slave_fd);

    // 父进程可以通过master_fd与新的bash进行交互
    // 关闭slave端
    close(slave_fd);
    // 父进程可以在这里通过master端与pty通信
    char buf[1024] = {0};
    ssize_t n = 0;
    n = read(master_fd, buf, sizeof(buf) - 1);
    if (n > 0)
    {
        buf[n] = 0;
        printf("[RD:%d]%s\n", n, buf);
    }
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf) - 1, stdin);
        if (5 == strlen(buf) && strcmp(buf, "exit"))
        {
            break;
        }
        n = write(master_fd, buf, strlen(buf));
        if (n > 0)
        {
            buf[n] = 0;
            printf("[IN:%d]%s", n, buf);
            n = read(master_fd, buf, sizeof(buf) - 1);
            if (n > 0)
            {
                buf[n] = 0;
                printf("%s\n", buf);
            }
        }

        sleep(1);
    }
    return 0;
}