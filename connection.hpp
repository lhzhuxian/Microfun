#ifndef _CONNECTION_H_
#define _CONNECTION_H_


#include "common.hpp"
#include <aio.h>

#define BUFFSIZE 2000
#define BLOCKSIZE 3
#define MAXDATASIZE 65536
struct wrap{
  int fd;
  int method;
  int id;
  aiocb c;
};

struct request {
  int id;
  char * data;
};

struct buffer{
  bool stone;
  bool full;
  void * buf;
};
int on_message_begin(http_parser* parser);
int on_headers_complete(http_parser* parser);
int on_message_complete(http_parser* parser) ;
int on_body(http_parser* parser, const char* buf, size_t len);
int on_url(http_parser* parser, const char* buf, size_t len);
int on_header_field(http_parser* parser, const char* buf, size_t len);



class Connection {
 
  int fd;
  int kq;
  wrap rblock[3];
  wrap wblock[3];
  int rav[3];
  int wav[3];
  atomic<int> request_id;
  atomic<int> responsed;
  priority_queue<http_response> waitinglist;
  http_request * remain;
  int cb;
  http_parser_settings setting_null;
  buffer b[3];
public:
  Connection(int f, int k);
  ~Connection();
  void Prepare_IO(aiocb *block, int i);
  int Check_status(int method, int id);
  void Receive_block(int method, int id);
  void Send_back(http_response response);
  void Start();
  void Deal(int id);
};


#endif
