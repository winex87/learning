#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/types.h>

using namespace std;

#define BUF_SZ          1024
#define NUM_MAX_EVENT   128

struct worker_params {
    int epollfd;
    int connfd;
    int ret;
};

int set_nonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void add_read_event(int epollfd, int fd, bool oneshot)
{
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    if (oneshot)
        event.events |= EPOLLONESHOT;
    int res = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    if (res < 0)
        cout << "epoll_ctl call exception" << endl;
    set_nonblock(fd);
}

void reset_oneshot(int epollfd, int fd)
{
    
    epoll_event event;
    event.events = EPOLLIN | EPOLLONESHOT | EPOLLET;
    event.data.fd = fd;
    int res = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    if (res < 0)
        cout << "epoll_ctl call exception" << endl;
}

void* handle_read_worker(void* arg) 
{
    int epollfd = ((worker_params*)arg)->epollfd;
    int fd = ((worker_params*)arg)->connfd;
    
    char buf[BUF_SZ];
    while (true) {
        memset(buf, '\0', BUF_SZ);
        int read_size = recv(fd, buf, BUF_SZ - 1, 0);
        if (read_size == 0) {
            cout << "server closed" << endl;
            ((worker_params*)arg)->ret = -1;
            break;
        }
        else if (read_size < 0) {
            if (errno != EAGAIN)
                ((worker_params*)arg)->ret = -1;
            // else
            //     reset_oneshot(epollfd, fd);
            break;
        }
        else {
            cout << endl;
            cout << "Msg from server : ";
            cout << buf << endl;
        }
    }
    pthread_exit(0);
}

void* handle_send_worker(void* arg)
{
    worker_params* param = (worker_params*)arg;

    while (true) {
        string msg;  
        if (!(cin >> msg)) {
            cout << "Bad input, please retry." << endl;
            cin.clear();
            continue;
        }

        if (msg == "q") {
            param->ret = -1;
            // close(param->connfd);
            break;
        }
        send(param->connfd, msg.c_str(), msg.size(), 0);
    }
    pthread_exit(0);
}

int connect_to(const char* argv[])
{
    const char* serv_ip = argv[1];
    int serv_port = atoi(argv[2]);
    
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(serv_port);
    int sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock_fd >= 0);
    
    if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" << endl;
        return -1;
    }
    return sock_fd;
}

int main(int argc, const char* argv[])
{
    // creat a socket connect to argv 
    int sock_fd = connect_to(argv);
    if (sock_fd < 0)
        return -1;

    int epollfd = epoll_create(5);
    add_read_event(epollfd, sock_fd, false);
    struct epoll_event events[NUM_MAX_EVENT];

    // threads parameter (shared by all threads)
    worker_params params;
    params.epollfd = epollfd;
    params.connfd = sock_fd;
    params.ret = 0;

    pthread_t thread_send;
    pthread_t thread_read;

    pthread_create(&thread_send, NULL, handle_send_worker, (void*)&params);

    while (params.ret == 0) {
        int num_event = epoll_wait(epollfd,  events, NUM_MAX_EVENT, 1000);
        if (num_event < 0) {
            cout << "epoll_wait call failure" << endl;
            break;
        } 
        // create pthread to handle the reading events
        if (num_event) 
            pthread_create(&thread_read, NULL, handle_read_worker, (void*)&params);
    }

    close(sock_fd);
    return 0;
}

