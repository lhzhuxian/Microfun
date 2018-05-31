#include "common.hpp"
#include "connection.hpp"

#define BACKLOG 20

typedef struct kevent kcb;

int max_event_count = 50;

int listener = 0;
int kq = 0;

unordered_map<int , unique_ptr<Connection> > connections;

void Accept(int kq, int size) {
  for (int i = 0; i < size; i++) {
    int client = accept(listener, NULL, NULL);
    if (client == -1) {
      perror("Accept failed");
    }

    connections[client] = unique_ptr<Connection>(new Connection(client, kq));
    connections[client]->Start();
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
      wrap* tmp = static_cast<wrap*> (events[i].udata);

      connections[tmp->fd]->Receive_block(tmp->method, tmp->id);
    }
  }
}





int main(void) {
  // set up listener
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
  listen(listener, BACKLOG);

  // set up kqueue

  kq = kqueue();
  if(kq == -1){
    err(EXIT_FAILURE, "kqueue() failed");
  }
  kcb change;
  EV_SET(&change, listener, EVFILT_READ, EV_ADD, 0, 0, NULL);
  
  int ret = kevent(kq, &change, 1, NULL, 0, NULL);
  if (ret == -1) {
    err(EXIT_FAILURE, "kevent register");
  }
  if (change.flags & EV_ERROR) {
    errx(EXIT_FAILURE, "Event error: %s", strerror(change.data));
  }

  kcb * events = static_cast<kcb *> (malloc(max_event_count * sizeof(kcb)));

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
