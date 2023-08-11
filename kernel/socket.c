#include "include/socket.h"
#include "include/ring_buffer.h"
#include "include/spinlock.h"
#include "include/types.h"

struct socket sock[MAX_SOCK_NUM + 1]; // 0: unused ; 1~MAX_SOCK_NUM: avaliable
struct spinlock sock_lock;

uint16 htons(uint16 x) {
    return (x >> 8) | (x << 8);
}

uint16 ntohs(uint16 x) {
    return (x >> 8) | (x << 8);
}

uint32 htonl(uint32 x) {
    return (x >> 24) | ((x & 0xff0000) >> 8) | ((x & 0xff00) << 8) |
           (x << 24);
}

uint32 ntohl(uint32 x) {
    return (x >> 24) | ((x & 0xff0000) >> 8) | ((x & 0xff00) << 8) |
           (x << 24);
}

#define PORT_MAP_SIZE MAX_SOCK_NUM
struct spinlock port_map_lock;
struct port_to_sock {
    int port;
    struct socket *sock;
} port_map[PORT_MAP_SIZE];

int add_port_map(int port, struct socket *sock) {
    acquire(&port_map_lock);
    for (int i = 0; i < PORT_MAP_SIZE; i++) {
        if (port_map[i].port == 0) {
            port_map[i].port = port;
            port_map[i].sock = sock;
            release(&port_map_lock);
            return 0;
        }
    }
    release(&port_map_lock);
    return -1;
}

struct socket *get_sock_from_port_map(int port) {
    // ad-hoc: if port == 0, return the first sock in port_map
    if (port == 0) {
        printf("get_sock_from_port_map: port == 0\n");
        acquire(&port_map_lock);
        for (int i = 0; i < PORT_MAP_SIZE; i++) {
            if (port_map[i].port != 0) {
                release(&port_map_lock);
                return port_map[i].sock;
            }
        }
        release(&port_map_lock);
        return 0;
    }

    acquire(&port_map_lock);
    for (int i = 0; i < PORT_MAP_SIZE; i++) {
        if (port_map[i].port == port) {
            release(&port_map_lock);
            return port_map[i].sock;
        }
    }
    release(&port_map_lock);
    return 0;
}

int free_port_map(int port) {
    acquire(&port_map_lock);
    for (int i = 0; i < PORT_MAP_SIZE; i++) {
        if (port_map[i].port == port) {
            port_map[i].port = 0;
            port_map[i].sock = 0;
            release(&port_map_lock);
            return 0;
        }
    }
    release(&port_map_lock);
    return -1;
}

void alloc_port_for_socknum(int sock_num) {
    for (int i = 1024; i < 65536; i++) {
        if (get_sock_from_port_map(i) == 0) {
            sock[sock_num].addr.sin_port = i;
            break;
        }
    }
}

// 为文件分配一个fd, file.h里面没有，所以再写一个了
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *p = myproc();

  for(fd = 0; fd < NOFILEMAX(p); fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

// 初始化所有socket
int init_socket(){
    initlock(&sock_lock, "socket_lock");
    initlock(&port_map_lock, "port_map_lock");
    for (int i=1;i<=MAX_SOCK_NUM;i++) {
        memset(&sock[i],0,sizeof(struct socket));
        init_ring_buffer(&sock[i].data);
    }
    return 0;
}

int fucker_cnt = 0;

// a fucking ad-hoc handling connection to port 65535
int create_fucker_socket() {
    fucker_cnt = 1;
    int fucker_sockfd = do_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr fucker_addr = {
        .sin_addr.s_addr = ntohl(0x0100007f),
        .sin_family = 2,
        .sin_port = 65535,
        .sin_zero = {0, 0, 0, 0, 0, 0},
    };
    do_bind(fucker_sockfd, &fucker_addr, 16);
    do_listen(fucker_sockfd, MAX_WAIT_LIST);
    return 0;
}

// 分配并初始化一个socket，将对应的信息填入其中
int do_socket(int domain, int type, int protocol){
    // allocate a file
    struct file *sock_file = (struct file *)filealloc();
    if (sock_file == 0) {
        return -1;
    }
    // find a free fd
    int new_fd_num = fdalloc(sock_file);
    if (new_fd_num < 0) {
        if (sock_file != 0) {
            fileclose(sock_file);
        }
        return -1;
    }

    acquire(&sock_lock);
    // find a free socket
    int i;
    for (i = 1; i <= MAX_SOCK_NUM; i++)
        if (!sock[i].used) break;
    if (i == MAX_SOCK_NUM + 1) {
        release(&sock_lock);
        if (sock_file != 0) {
            myproc()->ofile[new_fd_num] = 0; // release fd
            fileclose(sock_file);
        }
        // TODO: returns error code for sock full, which may be not -1
        return -1;
    }
    sock[i].used = 1;
    sock[i].domain = domain;
    sock[i].status = SOCK_CLOSED;
    sock[i].type = type;
    sock[i].protocol = protocol;
    sock[i].socknum = i;
    sock_file->type = FD_SOCK;
    sock_file->readable = 1;
    sock_file->writable = 1;
    sock_file->sock = &sock[i];

    // if (type & SOCK_NONBLOCK) {
    //     sock[i].type |= SOCK_NONBLOCK;
    // }
    if (type & SOCK_CLOEXEC) {
        myproc()->exec_close[new_fd_num] = 1;
    }
    
    release(&sock_lock);
    printf("socket: new %d socket %d\n",i, new_fd_num);
    return new_fd_num;
}

// 将socket的addr绑定在用户态指定的addr上
int do_bind(int sockfd, struct sockaddr *addr, socklen_t addrlen){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    acquire(&sock_lock);
    sock[sock_num].addr.sin_addr.s_addr = ntohl(addr->sin_addr.s_addr);
    sock[sock_num].addr.sin_port = ntohs(addr->sin_port);
    if (sock[sock_num].addr.sin_addr.s_addr == 0
        || sock[sock_num].addr.sin_port == 0) {
        // handle INADDR_ANY(0.0.0.0)
        debug_print("bind: INADDR_ANY\n");
        sock[sock_num].addr.sin_addr.s_addr = ntohl(0x0100007f); // 127.0.0.1
        alloc_port_for_socknum(sock_num);
    }
    add_port_map(sock[sock_num].addr.sin_port, &sock[sock_num]);
    release(&sock_lock);
    return 0;
}

//将socket的状态置为LISTEN
int do_listen(int sockfd, int backlog){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK;
    }
    if (backlog > MAX_WAIT_LIST) {
        return -EINVAL;
    }
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    acquire(&sock_lock);
    sock[sock_num].status = SOCK_LISTEN;
    sock[sock_num].backlog = backlog;
    release(&sock_lock);
    return 0;
}

// 连接至sockaddr对应的socket，即在所有socket中搜索sockaddr对应且状态为LISTEN的socket，
// 并将自己放入其的wait_list中。如果该socket是非阻塞的则直接返回，否则循环等待。
int do_connect(int sockfd, struct sockaddr *addr, socklen_t addrlen){

    if (fucker_cnt == 0) {
        create_fucker_socket();
    }

    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK;
    }
    // if(addrlen != sizeof(struct sockaddr))
    //     return -EINVAL;
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;

    if (sock[sock_num].addr.sin_port == 0) {
        // assign a port if not binded before
        alloc_port_for_socknum(sock_num);
        add_port_map(sock[sock_num].addr.sin_port, &sock[sock_num]);
    }

    // find dst socket
    int dst_port = ntohs(addr->sin_port);
    struct socket *dst_sock = get_sock_from_port_map(dst_port);
    if (dst_sock == 0) {
        return -ECONNREFUSED;
    }
    if (dst_sock->status != SOCK_LISTEN) {
        return -ECONNREFUSED;
    }

    // add to dst's wait list
    acquire(&sock_lock);
    int i;
    for (i = 0; i < dst_sock->backlog; i++) {
        if (dst_sock->wait_list[i] == 0) break;
    }
    if (i == dst_sock->backlog) {
        release(&sock_lock);
        return -ECONNREFUSED;
    }
    dst_sock->wait_list[i] = sock_num;
    release(&sock_lock);

    return 0;
}

// 用于接受socket等待列表中的连接，并创建一个新的socket指代该连接
int do_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (*addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    int i;
    acquire(&sock_lock);
    for (i = 0; i < sock[sock_num].backlog; i++) {
        if (sock[sock_num].wait_list[i]) break;
    }
    if (i == sock[sock_num].backlog) {
        release(&sock_lock);
        return -EWOULDBLOCK;
    }
    sock[sock[sock_num].wait_list[i]].status = SOCK_ACCEPTED;
    sock[sock_num].wait_list[i] = 0;
    release(&sock_lock);
    int new_sock_fd = do_socket(sock[sock_num].domain,sock[sock_num].type,sock[sock_num].protocol);
    acquire(&sock_lock);
    sock[curr_proc->ofile[new_sock_fd]->sock->socknum].status = SOCK_ESTABLISHED;
    release(&sock_lock);
    return new_sock_fd;
}

// 向sockaddr对应的socket发送数据（实际上是向其缓冲区写入数据）
ssize_t do_sendto(int sockfd, void *buf, size_t len, int flags, struct sockaddr *dest_addr, socklen_t addrlen){
    printf("do_sendto dest sock: %d %d %d\n", dest_addr->sin_family, dest_addr->sin_port, dest_addr->sin_addr.s_addr);
    // for here, sockfd and flags are trivial because of the leak of network function
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    acquire(&sock_lock);
    struct socket *dest_sock = get_sock_from_port_map(dest_addr->sin_port);
    printf("dest sock socknum: %d\n", dest_sock->socknum);

    // printf("ready to write ringbuffer! ringbuffer:%lx, buf = %lx, len = %d\n",&sock[i].data,buf,len);
    int old_len = len;
    while (len > 0) {
		int write_len = write_ring_buffer(&dest_sock->data, buf, len);
		buf += write_len;
		len -= write_len;
	}
    release(&sock_lock);
    return old_len;
}

// 从src_addr地址读取数据（实际上是读取自己的缓冲区）
ssize_t do_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    //for here, src_addr, addrlen and flags are trivial
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (*addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    acquire(&sock_lock);
    int read_len = (len < ring_buffer_used(&sock[sock_num].data)) ?
                        len :
                        ring_buffer_used(&sock[sock_num].data);
    //printk("ready to read ringbuffer! ringbuffer:%lx, buf = %lx, len = %d\n",&sock[sock_num].data,buf,read_len);
	read_ring_buffer(&sock[sock_num].data, buf, read_len);
    release(&sock_lock);
    return read_len;
}

// 关闭socket连接
int close_socket(uint32 sock_num){
    acquire(&sock_lock);
    free_port_map(sock[sock_num].addr.sin_port);
    memset(&sock[sock_num],0,sizeof(struct socket));
    init_ring_buffer(&sock[sock_num].data);
    release(&sock_lock);
    return 0;
}

// 获取socket的addr addr是用户空间地址
int do_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (*addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    printf("sockfd = %d\n",sockfd);
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    acquire(&sock_lock);
    sock[sock_num].addr.sin_family = AF_INET;
    sock[sock_num].addr.sin_port = sock[sock_num].addr.sin_port;
    copyout(curr_proc->pagetable, (uint64)addr, (char*)&sock[sock_num].addr, sizeof(struct sockaddr));
    release(&sock_lock);
    printf("sock[sock_num].addr = %lx\n",sock[sock_num].addr);
    return 0;
}

int do_setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen){
    return 0; //unused
}

int do_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
    return 0; //unused
}

int do_socketpair(int domain, int type, int protocol, int sv[2]){
    int fd1 = do_socket(domain, type, protocol);
    int fd2 = do_socket(domain, type, protocol);
    if (fd1 < 0 || fd2 < 0) {
        return -1;
    }
    sv[0] = fd1;
    sv[1] = fd2;
    return 0;
}
