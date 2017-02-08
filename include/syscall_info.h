#ifndef __SYSCALL_INFO_H
#define __SYSCALL_INFO_H

#include <sys/types.h>

typedef struct {
    pid_t pid;
    int syscall_nr;
} pro_con_bfr;

#endif
