# Initial process that execs all.
# This code runs in user space.

#include "include/sysnum.h"

# exec(init, argv)
.globl start
start:
        li a0, 2    //O_RDWR
        la a1, 1    //console
        li a2, 0
        li a7, SYS_dev
        ecall	# dev(O_RDWR, CONSOLE, 0);
        
        li a0, 0
        li a7, SYS_dup
        ecall	# dup(0);  // stdout

        li a0, 0
        li a7, SYS_dup
        ecall	# dup(0);  // stderr
        
        la a0, busybox
        la a1, argv_busybox
        li a7, SYS_exec
        ecall

loop:
        jal loop

# for(;;) exit();
program_exit:
        li a7, SYS_exit
        ecall
        jal program_exit

busybox:
        .string "/busybox\0"
param:
        .string "du\0"
argv_busybox:
        .dword busybox
        .dword param
        .dword 0 //不知道为什么要填充12个字节，实际上才能让编译出来的代码填充了8个0字节。



