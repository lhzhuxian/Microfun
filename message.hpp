#include <aio.h>
#include <memory.h>
struct Message: aiocb {
  Message(int kq);
  
}
