#include "connection.hpp"

Connection_wrap::Connection_wrap(int m, int i, Connection * cn): method(m), id(i), c(cn){}
Connection::Connection(int f, int k):fd(fd), kq(k), current(0), responsed(0) {
  for (int i = 0; i < 3; ++i) {
    Prepare_IO(&rblock[i], 0);
    Prepare_IO(&wblock[i], 1);
  }
}
Connection::~Connection() {
  for (int i = 0; i < 3; ++i) {
    free(rblock[i]);
    free(wblock[i]);
  }
  free(data);
}

void Connection::Prepare_IO(aiocb *block, int i) {
  block.aio_sigevent.sigev_notify = SIGEV_KEVENT;
  block.aio_sigevent.sigev_notify_kqueue = kq;
  block.aio_sigevent.sigev_notify_kevent_flags = EV_ONESHOT;
 
  block.aio_reqprio = 0;
  block.aio_buf = malloc(BUFFSIZE+1);
  if(!block.aio_buf) perror("malloc");
  block.aio_offset = 0;
  block.aio_fildes = fd;
  if (i) {
    block.aio_lio_opcode = LIO_WRITE;
  } else {
    block.aio_lio_opcode = LIO_READ;
  }
  block->aio_nbytes = BUFFSIZE;

}

void Connection::start(Connection_wrap * wrap) {
  block.aio_sigevent.sigev_value.sival_ptr = wrap;
  lio_listio(LIO_NOWAIT, rblock, 3, NULL);
}
