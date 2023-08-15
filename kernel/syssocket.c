#include "include/sysnum.h"
#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/stat.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/sleeplock.h"
#include "include/file.h"
#include "include/pipe.h"
#include "include/fcntl.h"
#include "include/socket.h"
#include "include/syscall.h"
#include "include/string.h"
#include "include/printf.h"
#include "include/vm.h"
#include "include/mmap.h"
#include "include/sysinfo.h"

#include "lwip/sockets.h"


uint64
sys_socket(void) {
    int domain, type, protocol;
    if (argint(0, &domain) < 0) {
        printf("sys_socket: argint(0, &domain) < 0\n");
        return -1;
    }
    
    if (argint(1, &type) < 0) {
        printf("sys_socket: argint(1, &type) < 0\n");
        return -1;
    }
    
    if (argint(2, &protocol) < 0) {
        printf("sys_socket: argint(2, &protocol) < 0\n");
        return -1;
    }
    return do_socket(domain, type, protocol);
}


uint64
sys_bind(void) {
    int sockfd;
    struct sockaddr *addr;
    socklen_t addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_bind: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&addr) < 0) {
        printf("sys_bind: argaddr(1, (void *)&addr) < 0\n");
        return -1;
    }
    if (argint(2, (int*)&addrlen) < 0) {
        printf("sys_bind: argint(2, &addrlen) < 0\n");
        return -1;
    }
    // printf("sys_bind: sockfd:%d, addr:%p, addrlen:%d\n", sockfd, addr, addrlen);
    struct sockaddr in;
    if(copyin(myproc()->pagetable,(char*)&in, (uint64)addr, sizeof(struct sockaddr)) < 0)
        return -1;
    return do_bind(sockfd, &in, addrlen);
}

uint64
sys_listen(void) {
    int sockfd, backlog;
    if (argint(0, &sockfd) < 0) {
        printf("sys_listen: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argint(1, &backlog) < 0) {
        printf("sys_listen: argint(1, &backlog) < 0\n");
        return -1;
    }
    // debug_print("sys_listen sockfd:%d, backlog:%d\n", sockfd, backlog);
    return do_listen(sockfd, backlog);
}

uint64
sys_connect(void) {
    int sockfd;
    struct sockaddr *addr;
    socklen_t addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_connect: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&addr) < 0) {
        printf("sys_connect: argaddr(1, (void *)&addr) < 0\n");
        return -1;
    }
    if (argint(2, (int*)&addrlen) < 0) {
        printf("sys_connect: argint(2, &addrlen) < 0\n");
        return -1;
    }
    struct sockaddr in;
    if(copyin(myproc()->pagetable,(char*)&in, (uint64)addr, sizeof(struct sockaddr)) < 0)
        return -1;
    return do_connect(sockfd, &in, addrlen);
}

uint64
sys_accept(void) {
    int sockfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_accept: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&addr) < 0) {
        printf("sys_accept: argaddr(1, (void *)&addr) < 0\n");
        return -1;
    }
    if (argaddr(2, (void *)&addrlen) < 0) {
        printf("sys_accept: argaddr(2, (int *)addrlen) < 0\n");
        return -1;
    }
    struct sockaddr in;
    if(copyin(myproc()->pagetable,(char*)&in, (uint64)addr, sizeof(struct sockaddr)) < 0)
        return -1;
    socklen_t inlen;
    if(copyin(myproc()->pagetable,(char*)&inlen, (uint64)addrlen, sizeof(socklen_t)) < 0)
        return -1;
    return do_accept(sockfd, &in, &inlen);
}

uint64
sys_sendto(void) {
    int sockfd;
    void *buf;
    size_t len;
    int flags;
    struct sockaddr *dest_addr;
    socklen_t addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_sendto: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&buf) < 0) {
        printf("sys_sendto: argaddr(1, (void *)&buf) < 0\n");
        return -1;
    }
    if (argaddr(2, &len) < 0) {
        printf("sys_sendto: argint(2, &len) < 0\n");
        return -1;
    }
    if (argint(3, &flags) < 0) {
        printf("sys_sendto: argint(3, &flags) < 0\n");
        return -1;
    }
    if (argaddr(4, (void *)&dest_addr) < 0) {
        printf("sys_sendto: argaddr(4, (void *)&dest_addr) < 0\n");
        return -1;
    }
    if (argint(5, (int*)&addrlen) < 0) {
        printf("sys_sendto: argint(5, &addrlen) < 0\n");
        return -1;
    }
    printf("sys_sendto: sockfd:%d, buf:%p, len:%d, flags:%d, dest_addr:%p, addrlen:%d\n", sockfd, buf, len, flags, dest_addr, addrlen);
    struct sockaddr in;
    if(copyin(myproc()->pagetable,(char*)&in, (uint64)dest_addr, sizeof(struct sockaddr)) < 0)
        return -1;
    return do_sendto(sockfd, buf, len, flags, &in, addrlen);
}

uint64
sys_recvfrom(void) {
    int sockfd;
    void *buf;
    size_t len;
    int flags;
    struct sockaddr *src_addr;
    socklen_t *addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_recvfrom: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&buf) < 0) {
        printf("sys_recvfrom: argaddr(1, (void *)&buf) < 0\n");
        return -1;
    }
    if (argaddr(2, &len) < 0) {
        printf("sys_recvfrom: argint(2, &len) < 0\n");
        return -1;
    }
    if (argint(3, &flags) < 0) {
        printf("sys_recvfrom: argint(3, &flags) < 0\n");
        return -1;
    }
    if (argaddr(4, (void *)&src_addr) < 0) {
        printf("sys_recvfrom: argaddr(4, (void *)&src_addr) < 0\n");
        return -1;
    }
    if (argaddr(5, (void *)&addrlen) < 0) {
        printf("sys_recvfrom: argint(5, (int *)addrlen) < 0\n");
        return -1;
    }
    struct sockaddr in;
    if(copyin(myproc()->pagetable,(char*)&in, (uint64)src_addr, sizeof(struct sockaddr)) < 0)
        return -1;
    socklen_t inlen;
    if(copyin(myproc()->pagetable,(char*)&inlen, (uint64)addrlen, sizeof(socklen_t)) < 0)
        return -1;
    return do_recvfrom(sockfd, buf, len, flags, &in, &inlen);
}

uint64
sys_getsockname(void) {
    //libctest的socket在这里的调用addr会是0，很奇怪
    return 0;
    int sockfd;
    uint64 addr = -1;
    socklen_t addrlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_getsockname: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argaddr(1, (void *)&addr) < 0) {
        printf("sys_getsockname: argaddr(1, (void *)&addr) < 0\n");
        return -1;
    }
    if (argaddr(2, (void *)&addrlen) < 0) {
        printf("sys_getsockname: argint(2, (int *)addrlen) < 0\n");
        return -1;
    }
    printf("sys_getsockname: addr = %p, addrlen = %p\n", addr, addrlen);
    return do_getsockname(sockfd, (struct sockaddr *)addr, &addrlen);
}

uint64
sys_setsockopt(void) {
    int sockfd;
    int level;
    int optname;
    void *optval;
    socklen_t optlen;
    if (argint(0, &sockfd) < 0) {
        printf("sys_setsockopt: argint(0, &sockfd) < 0\n");
        return -1;
    }
    if (argint(1, &level) < 0) {
        printf("sys_setsockopt: argint(1, &level) < 0\n");
        return -1;
    }
    if (argint(2, &optname) < 0) {
        printf("sys_setsockopt: argint(2, &optname) < 0\n");
        return -1;
    }
    if (argaddr(3, (void *)&optval) < 0) {
        printf("sys_setsockopt: argaddr(3, (void *)&optval) < 0\n");
        return -1;
    }
    if (argint(4, (int*)&optlen) < 0) {
        printf("sys_setsockopt: argint(4, &optlen) < 0\n");
        return -1;
    }
    return do_setsockopt(sockfd, level, optname, optval, optlen);
}