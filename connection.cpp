#include "connection.hpp"
#include "request.hpp"
#include "http_parser.h"


int on_message_begin(http_parser* parser) {
  parser->data = new http_request()
  return 0;
}

int on_headers_complete(http_parser* parser) {
  if (parser->content_length > MAXDATASIZE) //handler();
  return 0;
}

int on_message_complete(http_parser* parser) {
  http_request * request = static_cast<http_request *> (parser->data);
  parser->data = NULL;
  handler(request);
  return 0;
}
int on_body(http_parser* parser, const char* buf, size_t len){
  http_request * request = static_cast<http_request *> (parser->data);
  request->set_data(buf, len);
  
}

int on_url(http_parser* parser, const char* buf, size_t len) {
  http_request * request = static_cast<http_request *> (parser->data);
  request->set_url(buf, len);
  request->set_method(static_cast<char *> (http_method_str(parser->method)));
  return 0;
}

int on_header_field(http_parser* parser, const char* buf, size_t len) {
  http_request * request = static_cast<http_request *> (parser->data);
  request->set_headers(buf, len);
  printf("Header field: %.*s\n", (int)len, buf);
  return 0;
}

int on_header_value(http_parser* parser, const char* buf, size_t len) {
  http_request * request = static_cast<http_request *> (parser->data);
  request->set_values(buf, len);
  printf("Header value: %.*s\n", (int)len, buf);
  
  return 0;
}

Ring_buffer::Ring_buffer() {
  for (int i = 0; i < BLOCKSIZE; ++i) {
    ring[i] = NULL;
    buf[i] = malloc(BUFFSIZE + 1);
  }
  stone = 0;
}


Ring_buffer::~Ring_buffer() {
  for (int i = 0; i < BLOCKSIZE; ++i) {
    free(buf[i]);
  }
}
int Ring_buffer::is_null(int i) {
  if (ring[i] == null) {
    return 1;
  } else {
    return 0;
  }
}

int Ring_buffer::is_stone(int i) {
  int tmp = stone.load();
    if(tmp % 3 == i) {
      return 1;
    } else {
      return 0;
    }
}
int Ring_buffer::add_stone() {
  stone.fetch_add(1, memory_order_relaxed);
}

int Ring_buffer::next(int i) {
  i = i + 1;
  if (i >= BLOCKSIZE) {
    i = 0;
  }
  return i;
}

void * Ring_buffer::operator[](int i) {
  return ring[i];
}

void Ring_buffer::bufcpy(void * data, int len, int id) {
  memcpy(buf[id], data, len);
  ring[id] = buf[id];
}

void Ring_buffer::set_null(int id) {
  ring[id] = null;
}

Connection::Connection(int f, int k): fd(f), kq(k), request_id(0), rav({0, 0, 0}), wav({0, 0, 0}) {
  
  for (int i = 0; i < 3; ++i) {
    rblock[i].method = 0;
    rblock[i].id = i;
    rblock[i].fd = f;
    wblock[i].method = 1;
    wblock[i].id = i;
    wblock[i].fd = f;
    Prepare_IO(&rblock[i].c, 0);
    Prepare_IO(&wblock[i].c, 1);
  }
  
  setting_null.on_message_begin = on_message_begin;
  setting_null.on_header_field = on_header_field;
  setting_null.on_header_value = on_header_value;
  setting_null.on_url = on_url;
  setting_null.on_status = 0;
  setting_null.on_body = on_body;
  setting_null.on_headers_complete = on_headers_complete;
  setting_null.on_message_complete = on_message_complete;

}
Connection::~Connection() {
  for (int i = 0; i < 3; ++i) {
    free(const_cast<void*>(rblock[i].c.aio_buf));
    free(const_cast<void*>(wblock[i].c.aio_buf));
  }
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

void Connection::Receive_block(int method, int id) {
  if (Check_status(method, id) == -1) {
    perror("AIO error:");
    return;
  }
  if(!method) {
    ring.bufcpy(rblock[i].c->aio_buf, BUFFSIZE + 1, id);
    // need to think about how to read in order

    aio_read(&rblock[id].c);
    unique_lock <mutex> mlock(r_mutex);
    if(mlock.try_lock()) {
      if (ring.is_stone(id)) {
	while(ring[id]) {
	  Deal(id);
	  id = ring.next(id);
	  ring.add_stone();
	}
      }
    }
  } else {
    wav[id] = 1;
    wblock[id].len = BUFFSIZE;
    if (responsed == request_id && remain.buf) {
      Send(remain.buf, remain.offset, remain.len, id);
    }
  }
  
}

void Connection::Deal(int id) {
  const char * tmp =  const_cast<const char *> (static_cast<volatile char *> (rblock[id].c.aio_buf));

  // http parser
  http_parser * parser = new http_parser;
  http_parser_init(parser, HTTP_REQUEST);
  size_t parserd = http_parser_execute(parser, &setting_null, tmp, strlen(tmp));
  
  // http_parser();

    if (parser->data) {
      remain = static_cast<http_request *> (parser->data);
      parser->data == NULL;
    }
    ring.set_null(id);
}


int Connection::Send(void * data, int offset, int len, int id){
  int send = 0;
  if(wblock[i].len >= len) {
    memcpy(const_cast<void*>(wblock[i].c.aio_buf),	   \
	   data + offset,						   \
	   len);
    wblock[i].len -= len;
    if(responsed == request_id - 1) {
      aio_write(&wblock[i].c);
      wav[i] = 0;
      send = 1;
    }
    free(data);
    remain.buf = null;
  } else {
     memcpy(const_cast<void*>(wblock[i].c.aio_buf),        \
         data + offset,                                           \
         wblock[i].len);
     int tmp = wblock[i].len;
     aio_write(&wblock[i].c);
     wav[i] = 0;
     remain.buf = data;
     remain.offset = offset + tmp;
     remain.len -= tmp;
     send = 1;
  }
  if(send) {
    return 0;
  } else {
    return 1;
  }
}
void Connection::Send_back(http_response response) {
  waitinglist[response.id] = response;
  int id = responsed;
  unique_lock<mutex> mlock(s_mutex);
  if(mlock.try_lock()) {
    for (int i = 0; i < 3; ++i) {
      if (remain.buf) {
	Send(remain.buf, remain.offset, remain.len, i);
      }
      if (wav[i]) {
	while(waitinglist.find(responsed + 1)) {
	  send(waitinglist[responsed + 1].data, 0. waitinglist[responsed + 1].len, i);
	  responsed++;
	}
      }
    }
  }
}

