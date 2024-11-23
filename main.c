#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>  // 用于 basename
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LENGTH 1024

void getinfo(){

}

int main() {
    char cmd[MAX_CMD_LENGTH];

    while (1) {
        // 获取当前用户名
        char *user = getenv("USER");
        if (user == NULL) {
            user = "unknown";  // 如果无法获取用户名，使用默认值
        }

        // 获取当前主机名
        char host[1024];
        if (gethostname(host, sizeof(host)) == -1) {
            perror("gethostname failed");
            strcpy(host, "localhost");  // 如果无法获取主机名，使用默认值
        }

        // 获取当前工作目录
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd failed");
            strcpy(cwd, "/");  // 如果无法获取当前工作目录，使用默认值
        }

        // 处理当前目录路径，替换为 ~ 对于用户目录
        char cwd_display[1024];
        if (strncmp(cwd, getenv("HOME"), strlen(getenv("HOME"))) == 0) {
            // 当前目录在用户主目录下，替换为 ~
            cwd_display[0] = '~';
            strcpy(cwd_display + 1, cwd + strlen(getenv("HOME")));
        } else if (strcmp(cwd, "/") == 0) {
            // 根目录，显示 /
            strcpy(cwd_display, "/");
        } else {
            // 否则显示整个目录名
            strcpy(cwd_display, basename(cwd));  // 仅显示当前目录的文件夹名
        }

        // 显示提示符格式为: user@host current_folder_name %
        printf("%s@%s %s %% ", user, host, cwd_display);

        if (!fgets(cmd, MAX_CMD_LENGTH, stdin)) {
            break;
        }
        cmd[strcspn(cmd, "\n")] = '\0';  // 去掉换行符

        // 如果用户输入 "exit"，退出 shell
        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        // 如果用户输入 "cd" 命令，直接在当前进程中执行
        if (strcmp(cmd, "cd") == 0) {
            // 如果 cd 后没有参数，切换到用户主目录
            char *home = getenv("HOME");
            if (home == NULL) {
                fprintf(stderr, "HOME environment variable is not set\n");
            } else {
                if (chdir(home) != 0) {
                    perror("cd failed");
                }
            }
            continue;  // 继续等待下一个命令
        }

        // 如果用户输入 "cd" 后有路径参数，执行切换目录
        if (strncmp(cmd, "cd ", 3) == 0) {
            char *path = cmd + 3;  // 获取 "cd" 后面的路径部分
            if (strcmp(path, "~") == 0) {
                // 如果路径是 "~"，跳转到用户主目录
                char *home = getenv("HOME");
                if (home == NULL) {
                    fprintf(stderr, "HOME environment variable is not set\n");
                } else {
                    if (chdir(home) != 0) {
                        perror("cd failed");
                    }
                }
            } else {
                // 否则直接切换到指定路径
                if (chdir(path) != 0) {  // 改变当前工作目录
                    perror("chdir failed");
                }
            }
            continue;  // 继续等待下一个命令
        }

        // 创建子进程执行其他命令
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {  // 子进程
            // 解析命令字符串
            char *args[MAX_CMD_LENGTH / 2 + 1];
            char *token = strtok(cmd, " ");

            int i = 0;
            while (token != NULL) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;  // execvp 需要以 NULL 结尾

            // 执行命令
            execvp(args[0], args);

            // 如果 execvp 返回，表示命令执行失败
            perror("execvp failed");
            exit(1);  // 退出子进程
        } else {  // 父进程
            // 父进程等待子进程结束
            wait(NULL);
        }
    }

    return 0;
}