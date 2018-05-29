#ifndef _CONNECTION_H_
#define _CONNECTION_H_


#include "common.hpp"
#include <aio.h>

#define BUFFSIZE 2000
class wrap{
public:
  int fd;
  int method;
  int id;
  aiocb c;
};

struct request {
  int id;
  char * data;
};
class Connection {
 
  int fd;
  int kq;
  wrap rblock[3];
  wrap wblock[3];
  int available[3];
  void *data;
  //mutex cmutex;
  int current;
  int responsed;
public:
  Connection(int f, int k);
  ~Connection();
  void Prepare_IO(aiocb *block, int i);
  int Check_status(int method, int id);
  void Aread();
  void Awrite();
  void Start();
  void Deal(int method, int id);
};


#endif
