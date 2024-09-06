#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <utmp.h>
#include <sys/ioctl.h>
#include <iostream>
#include <pty.h>
 
int main() {
    int master;
    char *argv[3];
    char *env[1];
    pid_t pid;
    struct termios tty;
 
    // 创建新的pty
    if (openpty(&master, nullptr, nullptr, &tty, nullptr) == -1) {
        std::cerr << "无法打开新的pty" << std::endl;
        return 1;
    }
 
    // 设置 tty 为交互式
    tty.c_lflag |= ECHO | ICANON;
    tcsetattr(master, TCSANOW, &tty);
 
    // 设置环境变量
    env[0] = nullptr; // 表示环境变量列表结束
 
    // 设置 shell 参数
    argv[0] = const_cast<char*>("zsh"); // shell 名称
    argv[1] = const_cast<char*>("-i"); // 交互式标志
    argv[2] = nullptr; // 参数列表结束
 
    // 创建子进程
    pid = fork();
    if (pid == -1) {
        std::cerr << "无法创建子进程" << std::endl;
        return 1;
    } else if (pid > 0) {
        // 父进程逻辑
        close(master); // 关闭主pty

        char buf[128]={0};
        write(master, "pwd\n", 4);
        sleep(5);
        read(master, buf, 128);
        printf("%s\n", buf);
        sleep(100);

        // ... 父进程逻辑代码
    } else {
        // 子进程逻辑
        setsid(); // 创建新会话
        close(0); // 关闭标准输入
        close(1); // 关闭标准输出
        close(2); // 关闭标准错误输出
        close(master); // 关闭主pty
        // 重定向从pty到标准输入输出
        dup(master + 1);
        dup(master + 1);
        dup(master + 1);
        // 执行 shell
        execv("/usr/bin/zsh", argv);
        _exit(1); // 如果 execve 失败，退出子进程
    }
 
    return 0;
}