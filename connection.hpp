#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "commom.hpp"
#include <aio.h>

#define BUFFSIZE 1000
struct Connection_wrap {
  int method;
  int id;
  Connection* c;
  Connection_wrap();
  
}
class Connection {
  int fd;
  int kq;
  aiocb rblock[3];
  aiocb wblock[3];
  void *data;
  //mutex cmutex;
  int current;
  int responsed;
public:
  Connection();
  ~Connection();
  void Check_status(int method, int id);
  void Aread();
  void Awrite();
  void Start(Connection_wrap * wrap);
}

#endif
