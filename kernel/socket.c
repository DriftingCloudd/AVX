#include "include/socket.h"


struct socket sock[MAX_SOCK_NUM + 1]; // 0: unused ; 1~MAX_SOCK_NUM: avaliable
struct spinlock sock_lock;

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
    for (int i=1;i<=MAX_SOCK_NUM;i++) {
        memset(&sock[i],0,sizeof(struct socket));
        init_ring_buffer(&sock[i].data);
    }
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
    sock[sock_num].addr = addr;
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

// 连接至sockaddr对应的socket，即在所有socket中搜索sockaddr对应且状态为LISTENED的socket，
// 并将自己放入其的wait_list中。如果该socket是非阻塞的则直接返回，否则循环等待。
int do_connect(int sockfd, struct sockaddr *addr, socklen_t addrlen){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK;
    }
    if(addrlen != sizeof(struct sockaddr)) return -EINVAL;
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    int i, j;
    acquire(&sock_lock);
    for (i=1;i<=MAX_SOCK_NUM;i++){
        // printk("sock[%d]: addr = %x, status = %d\n",i,sock[i].addr->sin_addr,sock[i].status);
        if (!memcmp(addr,sock[i].addr,sizeof(struct sockaddr))
            && sock[i].status == SOCK_LISTEN)
            break;
    }
    if (i == MAX_SOCK_NUM + 1) {
        release(&sock_lock);
        return -ECONNREFUSED;
    }
    for (j = 0; j < sock[i].backlog; j++)
        if (!sock[i].wait_list[j]) {
            sock[i].wait_list[j] = sock_num;
            break;
        }
    if (j == sock[i].backlog) {
        release(&sock_lock);
        return -EWOULDBLOCK;
    }
    if (sock[sock_num].type & SOCK_NONBLOCK){
        sock[sock_num].status = SOCK_ACCEPTED;
    }
    else {
        while (sock[sock_num].status != SOCK_ACCEPTED){
            release(&sock_lock);
            // do_scheduler();
            yield(); // give up cpu here
            acquire(&sock_lock);
        }
    }
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
    if (addr != sock[sock_num].addr) {
        return -EINVAL;
    }
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
    // for here, sockfd and flags are trivial because of the leak of network function
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    int i,j;
    acquire(&sock_lock);
    for (i = 1; i <= MAX_SOCK_NUM; i++)
        if (!memcmp(dest_addr, sock[i].addr, sizeof(struct sockaddr)) && 
            ((sock[i].protocol == IPPROTO_UDP) || (sock[i].protocol == IPPROTO_TCP && sock[i].status == SOCK_ESTABLISHED))) break;
    if (i == MAX_SOCK_NUM + 1) {
        release(&sock_lock);
        return -ENOTCONN;
    }
    //printk("ready to write ringbuffer! ringbuffer:%lx, buf = %lx, len = %d\n",&sock[i].data,buf,len);
    int old_len = len;
    while (len > 0) {
		int write_len = write_ring_buffer(&sock[i].data,buf, len);
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
    int i;
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
    memset(&sock[sock_num],0,sizeof(struct socket));
    init_ring_buffer(&sock[sock_num].data);
    release(&sock_lock);
    return 0;
}

// 获取socket的addr
int do_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    struct proc *curr_proc = myproc();
    if (curr_proc->ofile[sockfd]->type != FD_SOCK) {
        return -ENOTSOCK; 
    }
    if (*addrlen != sizeof(struct sockaddr)) {
        return -EINVAL;
    }
    int sock_num = curr_proc->ofile[sockfd]->sock->socknum;
    acquire(&sock_lock);
    memcpy(addr,sock[sock_num].addr,sizeof(struct sockaddr));
    release(&sock_lock);
    return 0;
}

int do_setsockopt(int sockfd, int level, int optname, void *optval, socklen_t optlen){
    return 0; //unused
}
