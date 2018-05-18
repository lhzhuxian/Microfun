#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <err.h>
#include <unordered_map>
#include <aio.h>
#include <signal.h>
#include <cstring>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string>

#define BUFFSIZE 100

using namespace std;
int max_event_count = 50;

int listener;
unordered_map<int, aiocb*> connection;
void Prepare_IO(aiocb * block, int fd, int kq){
  memset(block, 0, sizeof(aiocb));
  block->aio_sigevent.sigev_notify = SIGEV_KEVENT;
  block->aio_sigevent.sigev_notify_kqueue = kq;
  block->aio_sigevent.sigev_notify_kevent_flags = EV_ONESHOT;
  block->aio_sigevent.sigev_value.sival_ptr = block;
  block->aio_reqprio = 0;
  block->aio_buf = malloc(BUFFSIZE+1);
  if(!block->aio_buf) perror("malloc");
  block->aio_offset = 0;
  block->aio_fildes = fd;
  cout << fd << endl;
  //block->aio_lio_opcode = i32LioOp;
  block->aio_nbytes = BUFFSIZE;
}
void Accept(int kq, int size) {
  for (int i = 0; i < size; i++) {
    int client = accept(listener, NULL, NULL);
    if (client == -1) {
      perror("Accept failed");
    }

    aiocb * block = static_cast<aiocb*> (malloc(sizeof(aiocb)));
    connection[client] = block;
    Prepare_IO(block, client, kq);
    if(aio_read(connection[client]) < 0) perror("aio_read");
  }
}

void Handle_event(int kq, struct kevent * events, int nevents) {
  for (int i = 0; i < nevents; i++) {
    int sock = events[i].ident;
    int data = events[i].data;
    if (events[i].flags & EV_ERROR) {
      errx(EXIT_FAILURE, "Event error: %s", strerror(data));
    }
    
    if (sock == listener) {
      Accept(kq, data);
    } else {
      if (events[i].flags & EV_ERROR) {
	errx(EXIT_FAILURE, "Event error: %s", strerror(events[i].data));
      }
      aiocb* tmp = static_cast<aiocb*> (events[i].udata);
      int ret = aio_return(tmp);
      if (ret == -1) {
	perror("read failed");
      } else {
	string ans = static_cast<char *> (const_cast<void *> (tmp->aio_buf));
	cout << ans;
      }
      cout << "sock: " << sock << endl;
      if(aio_read(tmp) < 0) perror("aio_read");

    }
  }
}





int main(void) {
  // set up socket
  int status;
  addrinfo hints;
  addrinfo *servinfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, "9999", &hints, &servinfo)) != 0) {
    cerr << "getaddrinfo error : " << gai_strerror(status) << endl;
  }

  listener = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  bind(listener, servinfo->ai_addr, servinfo->ai_addrlen);
  listen(listener, 20);

  // set up kqueue

  int kq = kqueue();
  if(kq == -1){
    err(EXIT_FAILURE, "kqueue() failed");
  }
  struct kevent change;
  EV_SET(&change, listener, EVFILT_READ, EV_ADD, 0, 0, NULL);
  
  int ret = kevent(kq, &change, 1, NULL, 0, NULL);
  if (ret == -1) {
    err(EXIT_FAILURE, "kevent register");
  }
  if (change.flags & EV_ERROR) {
    errx(EXIT_FAILURE, "Event error: %s", strerror(change.data));
  }

  struct kevent * events = static_cast<struct kevent *> (malloc(max_event_count * sizeof(struct kevent)));
  while(true) {
    int kn = kevent(kq, NULL, 0, events, max_event_count, NULL);
    if(kn == -1) {
      err(EXIT_FAILURE, "kevent failed");
    }
    Handle_event(kq, events, kn);
  }
  
  
  
  free(events);
  freeaddrinfo(servinfo);
}
