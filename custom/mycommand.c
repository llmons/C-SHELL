//
// Created by Kongho Leung on 2024/12/2.
//

#include <unistd.h>
#include<sys/syscall.h>

#define SYS__mysyscall 0

int main() {
    syscall(SYS__mysyscall);
    return 0;
}
