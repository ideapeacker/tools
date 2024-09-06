import os
import pty
 
global pid__
def spawn_interactive_shell(program):
    """
    创建一个新的交互式 shell。
    参数:
    - program: 要执行的程序名称。
    """
    # 获取当前进程的进程组ID
    pid, fd = pty.fork()
    pid__ = pid 
    if pid == 0:  # 子进程
        # 用 execve 替换当前进程映像，运行 shell 程序
        os.execve(program, [], os.environ)  # 注意：这里的 os.environ 是一个字典，包含了当前环境变量
    else:  # 父进程
        # 返回 pty 的 master 文件描述符
        return fd
 
# 使用 spawn_interactive_shell 函数来创建一个新的交互式 shell
shell_fd = spawn_interactive_shell('/bin/bash')
 
# 通过文件描述符与交互式 shell 通信
os.write(shell_fd, b'ls -a\n')
 
# 读取 shell 的输出
output = os.read(shell_fd, 1024)
print('Output from shell:', output)
 
# 关闭文件描述符和 shell 进程
os.close(shell_fd)
#os.waitpid(pid__, 0)