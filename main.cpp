#include "common.hpp"
#include "connection.hpp"
#include "request.hpp"

#define BACKLOG 20
#define MAXTHREAD 4


int max_event_count = 50;

int listener = 0;
int kq = 0;

extern unordered_map<int , unique_ptr<Connection> > connections;

void Accept(int kq, int size) {
  // Accept the connection
  for (int i = 0; i < size; i++) {
    int client = accept(listener, NULL, NULL);
    if (client == -1) {
      perror("Accept failed");
    }

    connections[client] = unique_ptr<Connection>(new Connection(client, kq));
    connections[client]->Start();
  }
}

void Handle_event(int kq, kcb * events, int nevents) {
  for (int i = 0; i < nevents; i++) {
    int sock = events[i].ident;
    int data = events[i].data;
    if (events[i].flags & EV_ERROR) {
      cerr << "Event error: " << strerror(data) << endl;
    }

    
    if (sock == listener) {
      // there is a conneciton
      Accept(kq, data);
    } else {
           
      wrap* tmp = static_cast<wrap*> (events[i].udata);
      int ans = connections[tmp->fd]->Receive_block(tmp->method, tmp->id);
      if (!ans) connections.erase(connections.find(tmp->fd));
    }
  }
}



void Kfunction(){
  // IO multiplexing * AIO

  kcb events[max_event_count];
  thread::id this_id = this_thread::get_id();
  cout << "Thread: " << this_id << endl;

  while(true) {
    int kn = kevent(kq, NULL, 0, events, max_event_count, NULL);
    if(kn == -1) {
      perror("kevent failed");
      continue;
    }
    Handle_event(kq, events, kn);
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

  // set up kqueue and register listener

  
  if((kq = kqueue()) == -1){
    perror("kqueue() failed");
    exit(EXIT_FAILURE);
  }
  kcb change;
  EV_SET(&change, listener, EVFILT_READ, EV_ADD, 0, 0, NULL);
  
  int ret = kevent(kq, &change, 1, NULL, 0, NULL);
  if (ret == -1) {
    perror("Falied in register listener");
    exit(EXIT_FAILURE);
  }
  if (change.flags & EV_ERROR) {
    errx(EXIT_FAILURE, "Event error: %s", strerror(change.data));
  }

  // set up threads and start running

  vector<thread> threadpool;
  try {
    for (int i = 0; i < MAXTHREAD; i++) {
      threadpool.push_back(thread(Kfunction));
      threadpool[i].detach();
    }
    Kfunction();
  } catch (const exception & e) {
    freeaddrinfo(servinfo);
  }
}
