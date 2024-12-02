/*
 * by llmons 2024
 * a SHELL program
 * builtin command includes: cd, exit
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_HOST_LEN 1024
#define MAX_CWD_LEN 1024
#define MAX_CMD_LEN 1024

char *user;
char host[MAX_HOST_LEN];
char cwd[MAX_CWD_LEN];
char cmd[MAX_CMD_LEN];

void get_info();

char *trim(char *);

void condense_spaces(char *);

int main() {
    while (1) {
        get_info();

        // get command
        char line[MAX_CMD_LEN];
        if (fgets(line, MAX_CMD_LEN, stdin) == NULL)break;

        strcpy(cmd, trim(line));
        condense_spaces(cmd);
        if (strcmp(cmd, "exit") == 0)break;

        // handle cd command
        if (strcmp(cmd, "cd") == 0) {
            char *home = getenv("HOME");
            if (home == NULL) {
                fprintf(stderr, "HOME environment variable is not set\n");
            } else {
                if (chdir(home) != 0)perror("cd failed");
            }
            continue;
        }

        if (strncmp(cmd, "cd ", 3) == 0) {
            char *path = cmd + 3;
            if (strcmp(path, "~") == 0) {
                char *home = getenv("HOME");
                if (home == NULL) {
                    fprintf(stderr, "HOME environment variable is not set\n");
                } else {
                    if (chdir(home) != 0)perror("cd failed");
                }
            } else {
                if (chdir(path) != 0)perror("chdir failed");
            }
            continue;
        }

        // exec command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pid == 0) { // child
            char *args[MAX_CMD_LEN / 2 + 1];
            char *token = strtok(cmd, " ");

            int i = 0;
            while (token != NULL) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;
            execvp(args[0], args);

            perror("execvp failed");
            exit(1);
        } else {  // parent
            wait(NULL);
        }
    }
    return 0;
}

void get_info() {
    // get user
    user = getenv("USER");
    if (user == NULL) {
        user = "unknown";
    }

    // get hostname
    if (gethostname(host, sizeof(host)) == -1) {
        perror("get hostname failed");
        strcpy(host, "localhost");
    }

    // get current working dictionary
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("get cwd failed");
        strcpy(cwd, "/");
    }

    // handle cwd
    char cwd_display[MAX_CWD_LEN];
    if (strcmp(cwd, getenv("HOME")) == 0) {
        strcpy(cwd_display, "~\0");
    } else {
        strcpy(cwd_display, basename(cwd));
    }

    // show: user@host cwd %
    printf("%s@%s %s %% ", user, host, cwd_display);
}

char *trim(char *str) {
    while (isspace((unsigned char) *str))++str;
    if (*str == '\0')return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) --end;
    end[1] = '\0';
    return str;
}

void condense_spaces(char *str) {
    char *read = str, *write = str;
    int space_found = 0;

    while (*read != '\0') {
        if (isspace((unsigned char) *read)) {
            if (!space_found) {
                *write++ = ' ';
                space_found = 1;
            }
        } else {
            *write++ = *read;
            space_found = 0;
        }
        read++;
    }
    *write = '\0';
}