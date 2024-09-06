#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <utmp.h>
#include <pty.h>

int main(int argc, char *argv[])
{
    int master_fd, slave_fd;
    char *slave_name;
    pid_t pid;

    // 创建新的pty
    if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) == -1)
    {
        perror("openpty");
        exit(1);
    }

    slave_name = ttyname(master_fd);
    // slave_name = ptsname(master_fd);
    // 打印出slave名称
    printf("Slave name is %s\n", slave_name);

    // 创建子进程
    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    { // 子进程
        // 关闭master端
        close(master_fd);
        // 重新打开slave端，并连接到控制终端
        int slave = open(slave_name, O_RDWR);
        if (slave == -1)
        {
            perror("open");
            exit(1);
        }

        // 设置slave端的终端属性
        struct termios tty;
        tcgetattr(slave, &tty);
        tty.c_cflag |= CREAD | CLOCAL; // 开启接收字符
        tcsetattr(slave, TCSANOW, &tty);

        if (login_tty(slave_fd) == -1)
        {
            perror("login_tty");
            exit(1);
        }

        // 将slave端设置为当前的标准输入、输出、错误输出
        // dup2(slave_fd, 0);
        // dup2(slave_fd, 1);
        // dup2(slave_fd, 2);

        // 关闭不需要的文件描述符
        close(slave);

        // 执行shell
        static char *const argv[] = {"sh"};
        //execve("/bin/sh", 0, 0);
        execv("/bin/sh", 0);
        perror("execl");
        exit(1);
    }
    else
    { // 父进程
        // 关闭slave端
        close(slave_fd);
        // 父进程可以在这里通过master端与pty通信
        char buf[1024] = {0};
        ssize_t n = 0;
        n = read(master_fd, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = 0;
            printf("[RD:%d]%s", n, buf);
        }
        ssize_t len = 0;
        while (1)
        {
            memset(buf, 0, sizeof(buf));
            // len = fread(buf, sizeof(buf) - 1, sizeof(len) - 1, stdin);
            fgets(buf, sizeof(buf) - 1, stdin);
            len = strlen(buf);
            if (5 == len && strcmp(buf, "exit"))
            {
                break;
            }
            printf("ready write : %d\n", len);
            n = write(master_fd, buf, len);
            if (n > 0)
            {
                printf("write : %d\n", len);
                n = read(master_fd, buf, sizeof(buf) - 1);
                if (n > 0)
                {
                    buf[n] = 0;
                    printf("%s", buf);
                }
            }
        }
    }

    return 0;
}