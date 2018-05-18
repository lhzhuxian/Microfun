#include "message.hpp"

Message::Message(int kq, int bufsize) {
  memset(static_cast<aiocb *> (this), 0, sizeof(aiocb));
  aio_sigevent.sigev_notify = SIGEV_KEVENT;
  aio_sigevent.sigev_notify_kqueue = kq;
  aio_sigevent.sigev_notify_kevent_flags = EV_ONESHOT;
  //aio_sigevent.sigev_value.sival_ptr = pvContext;
  aio_reqprio = 0;
  //void * pvVirtualBuf = &m_sAddr;
  
  aio_buf = pvVirtualBuf;
  aio_offset = m_aucData - static_cast<unsigned char *>(pvVirtualBuf);
  aio_fildes = i32FileDesc;
  aio_lio_opcode = i32LioOp;
  aio_nbytes = aio_offset + stBufByte;

}
