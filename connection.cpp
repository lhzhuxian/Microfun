#include "connection.hpp"

#include "http_parser.h"


int on_message_begin(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE BEGIN***\n\n");
  return 0;
}

int on_headers_complete(http_parser* _) {
  (void)_;
  printf("\n***HEADERS COMPLETE***\n\n");
  return 0;
}

int on_message_complete(http_parser* _) {
  (void)_;
  printf("\n***MESSAGE COMPLETE***\n\n");
  cout << " messge length: " << _->nread << endl;
  return 0;
}

int on_url(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Url: %.*s\n", (int)length, at);
  return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header field: %.*s\n", (int)length, at);
  return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Header value: %.*s\n", (int)length, at);
  return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
  (void)_;
  printf("Body: %.*s\n", (int)length, at);
  return 0;
}



Connection::Connection(int f, int k) {
  fd = f;
  kq = k;
  current = 0;
  responsed = 0;
  for (int i = 0; i < 3; ++i) {
    rblock[i].method = 0;
    rblock[i].id = i;
    rblock[i].fd = f;
    wblock[i].method = 1;
    wblock[i].id = i;
    wblock[i].fd = f;
    Prepare_IO(&rblock[i].c, 0);
    Prepare_IO(&wblock[i].c, 1);
   
    available[i] = 1;
  }
}
Connection::~Connection() {
  for (int i = 0; i < 3; ++i) {
    free(const_cast<void*>(rblock[i].c.aio_buf));
    free(const_cast<void*>(wblock[i].c.aio_buf));
  }
  //free(data);
}

void Connection::Prepare_IO(aiocb *block, int i) {
  block->aio_sigevent.sigev_notify = SIGEV_KEVENT;
  block->aio_sigevent.sigev_notify_kqueue = kq;
  block->aio_sigevent.sigev_notify_kevent_flags = EV_ONESHOT;
 
  block->aio_reqprio = 0;
  block->aio_buf = malloc(BUFFSIZE+1);

  if(!block->aio_buf) perror("malloc");
  block->aio_offset = 0;
  block->aio_fildes = fd;
  
  if (i) {
    block->aio_lio_opcode = LIO_WRITE;
  } else {
    block->aio_lio_opcode = LIO_READ;
  }
  block->aio_nbytes = BUFFSIZE;

}

void Connection::Start() {
  
  for (int i = 0; i < 3; i++) {
    
    rblock[i].c.aio_sigevent.sigev_value.sival_ptr = &rblock[i];
    wblock[i].c.aio_sigevent.sigev_value.sival_ptr = &wblock[i];
    aio_read(&rblock[i].c);
  }
  /*
  if (lio_listio(LIO_NOWAIT, ans, 3, NULL) == -1) {
    perror("failed start");
    }*/
  
}

int Connection::Check_status(int method, int id) {
  if (method) {
    return aio_return(&wblock[id].c);
  } else {
    return aio_return(&rblock[id].c);
  }
}

void Connection::Deal(int method, int id) {
  int check;
  if ((check = Check_status(method, id)) == -1) {
    perror("AIO error:");
  }
  cout << "fd: " << fd << endl;
  cout << "method: " << method << endl;
  cout << "id: " << id << endl;
  cout << "return : " << check << endl;
  if (!method) {
    for (int i = 0; i < 3; ++i) {
      if (available[i]) {
	const char * tmp =  const_cast<const char *> (static_cast<volatile char *> (rblock[id].c.aio_buf));
	cout << "receive:" << strlen(tmp) << endl;
	cout << tmp << endl;
	// http parser
	http_parser * parser = static_cast<http_parser*> (malloc(sizeof(http_parser)));
	http_parser_init(parser, HTTP_REQUEST);
	http_parser_settings setting_null;
	setting_null.on_message_begin = on_message_begin;
	setting_null.on_header_field = on_header_field;
	setting_null.on_header_value = on_header_value;
	setting_null.on_url = on_url;
	setting_null.on_status = 0;
	setting_null.on_body = on_body;
	setting_null.on_headers_complete = on_headers_complete;
	setting_null.on_message_complete = on_message_complete;
	
	
	size_t parserd = http_parser_execute(parser, &setting_null, tmp, strlen(tmp));
	
	// http_parser();

	
	
	memcpy(const_cast<void*>(wblock[i].c.aio_buf),	\
	       tmp,	\
	       strlen(tmp));
	available[i] = 0;
	wblock[i].c.aio_nbytes = BUFFSIZE;
	rblock[i].c.aio_nbytes = BUFFSIZE;
	aio_write(&wblock[i].c);
	aio_read(&rblock[id].c);
	break;
      }
    }
  } else {
    available[id] = 1;
  }
}