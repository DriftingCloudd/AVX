#include "lwip/ip_addr.h"
#include "lwip/ip.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"

#include "include/vm.h"
#include "include/sem.h"
#include "include/kalloc.h"
#include "include/socket.h"
#include "include/spinlock.h"
#include "include/types.h"

static void
tcpip_init_done(void *arg) {
  volatile int *tcpip_done = arg;
  *tcpip_done = 1;
}



void tcpip_init_with_loopback(void) {
  // Initialize the lwIP stack
  volatile int tcpip_done = 0;
  tcpip_init(tcpip_init_done, &tcpip_done);
}

// 分配并初始化一个socket，将对应的信息填入其中
int do_socket(int domain, int type, int protocol){
    return lwip_socket(domain, type, protocol);
}

// 将socket的addr绑定在用户态指定的addr上
int do_bind(int sockfd, struct sockaddr *addr, socklen_t addrlen){
    return lwip_bind(sockfd, addr, addrlen);
}

//将socket的状态置为LISTEN
int do_listen(int sockfd, int backlog){
    return lwip_listen(sockfd, backlog);
}

// 连接至sockaddr对应的socket，即在所有socket中搜索sockaddr对应且状态为LISTEN的socket，
// 并将自己放入其的wait_list中。如果该socket是非阻塞的则直接返回，否则循环等待。
int do_connect(int sockfd, struct sockaddr *addr, socklen_t addrlen){
    return lwip_connect(sockfd, addr, addrlen);
}

// 用于接受socket等待列表中的连接，并创建一个新的socket指代该连接
int do_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    return lwip_accept(sockfd, addr, addrlen);
}

// 向sockaddr对应的socket发送数据（实际上是向其缓冲区写入数据）
// 注意：buf是用户空间地址！
ssize_t do_sendto(int sockfd, void *buf, size_t len, int flags, struct sockaddr *dest_addr, socklen_t addrlen){
    void *kbuf = kalloc();
    copyin(myproc()->pagetable, kbuf, (uint64)buf, len);
    ssize_t ret = lwip_sendto(sockfd, kbuf, len, flags, dest_addr, addrlen);
    kfree(kbuf);
    return ret;
}

// 从src_addr地址读取数据（实际上是读取自己的缓冲区）
// 注意：buf是用户空间地址！
ssize_t do_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    void *kbuf = kalloc();
    ssize_t ret = lwip_recvfrom(sockfd, kbuf, len, flags, src_addr, addrlen);
    copyout(myproc()->pagetable, (uint64)buf, kbuf, len);
    kfree(kbuf);
    return ret;
}

// 关闭socket连接
int close_socket(uint32 sock_num){
    return lwip_close(sock_num);
}

// 获取socket的addr addr是用户空间地址
int do_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    return lwip_getsockname(sockfd, addr, addrlen);
}

int do_setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen){
    // return lwip_setsockopt(sockfd, level, optname, optval, optlen); //unused
    return 0;
}

int do_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
    return lwip_getsockopt(sockfd, level, optname, optval, optlen); //unused
}

int do_socketpair(int domain, int type, int protocol, int sv[2]){
    // return lwip_socketpair(domain, type, protocol, sv); //unused
    return 0;
}
