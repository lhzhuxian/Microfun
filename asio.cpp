#include <aio.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <cstdlib>
#include <signal.h>

#define BUFSIZE 100;
using std::cerr;
vector<struct aiocb> aiotable;
int MAX_EVENT_COUNT = 0;
bool Register(int kq, int fd) {
  struct kevent changes[1];
  EV_SET(&changes[0], fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  int ret = kevent(kq, changes, 1, NULL, 0, NULL);
  MAX_EVENT_COUNT++;
  return true;
}

Void Accept(int sock, int kq, int size) {
  for (int i = 0; i < size; ++i) {
    int client = accept(sock, NULL, NULL);
    struct aiocb my_aiocb;
    memset(&my_aiocb, 0, sizeof my_aiocb);
    my_aiocb.aio_buf = malloc(BUFSIZE+1);
    if(!my_aiocb.aio_buf) perror("malloc");
    my_aiocb.aio_fildes = fd;
    my_aiocb.aio_nbytes = BUFSIZE;
    my_aiocb.aio_offset = 0;
    //My_aiocb.aio_sigevent.sigev_notify = SIGEV_KEVENT;
    //my_aiocb.aio_sigevent.sigev_notify_kevent_flags = EV_CLEAR;
    MAX_EVENT_COUNT++;
    
    ret = aio_read(&my_aiocb);
    if(ret < 0) perror("aio_read");
  }
}

int main() {
  // build server socket
  int status;
  struct addrinfo hints;
  struct addrinfo * servinfo;
  int sock;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, "9999", &hints, &servinfo)) != 0) {
    cer << "getaddrinfo error: " << gai_strerror(status) << endl;
    exit(-1);
  }
  sock = socket(servinfo->ai_family, servinfo->socktype, servinfo->protocol);
  bind(sock, servinfo->ai_addr, res->ai_addrlen);
  listen(sock, 20);

  
  int kq = kqueue();
  Register(kq, sock);
  struct kevent * events;
  events = malloc(MAX_EVENT_COUNT * sizeof(struct kevent));
  while(true) {
    int ret = kevent(kq, NULL, 0, events, MAX_EVENT_COUNT, NULL);
    for (int i = 0; i < nevents; i++) {
    int ident = events[i].ident;
    int data = events[i].data;
    if (ident == sock) {
      Accept(kq, data);
    } else {
    }

  }
  
  
  
  
  freeaddrinfo(servinfo);
}

  
