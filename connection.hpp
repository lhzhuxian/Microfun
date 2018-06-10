#ifndef _CONNECTION_H_
#define _CONNECTION_H_


#include "common.hpp"
#include <aio.h>
#include "request.hpp"

#define BUFFSIZE 2000
#define BLOCKSIZE 3
#define MAXDATASIZE 65536
struct wrap{
  int fd;
  int method;
  int id;
  int len;
  aiocb c;
};




class Ring_buffer{
  void * ring[BLOCKSIZE];
  void * buf[BLOCKSIZE];
  atomic<int> stone;
public:
  Ring_buffer();
  ~Ring_buffer();

  int is_stone(int i);
  void add_stone();
  int next(int i);
  void bufcpy(void * data, int len, int id);
  void set_null(int i);
  void * operator[](int i);
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

  int responsed;
  int rav[3];
  int wav[3];
  wrap rblock[3];
  wrap wblock[3];
  unordered_map<int, http_response> waitinglist;
  http_request * remain_request;
  http_parser_settings setting_null;
  Ring_buffer ring;
  mutex s_mutex;
  mutex r_mutex;
  http_response remain_response;
public:
  atomic<int> request_id;
  Connection(int f, int k);
  ~Connection();
  void Prepare_IO(aiocb *block, int i);
  int Check_status(int method, int id);
  void Receive_block(int method, int id);
  int Send(void*data, int offset, int len, int id);
  void Send_back(http_response response);
  void Start();
  void Deal(int id);
};

extern unordered_map<int, unique_ptr<Connection> > connections;
void handler(http_request * request, Connection * c);
#endif
