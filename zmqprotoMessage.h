#ifndef ZMQPROTOMESSAGE_H_
#define ZMQPROTOMESSAGE_H_

#include <cstddef>

#include <zmq.h>

class zmqprotoMessage
{
  public:
    zmqprotoMessage();
    zmqprotoMessage(size_t size);
    zmqprotoMessage(void* data, size_t size);

    virtual void* GetMessage();
    virtual void* GetData();
    virtual size_t GetSize();

    virtual void CloseMessage();

    static void CleanUp(void* data, void* hint);

    virtual ~zmqprotoMessage();

  private:
    zmq_msg_t fMessage;
};

#endif
