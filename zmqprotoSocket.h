#ifndef ZMQPROTOSOCKET_H
#define ZMQPROTOSOCKET_H

#include <cstdlib>
#include <string>

#include <zmq.h>

class zmqprotoSocket
{
  public:
    zmqprotoSocket(void* fContext, const std::string& type, int num);
    
    virtual unsigned long GetBytesTx();
    virtual unsigned long GetBytesRx();
    virtual unsigned long GetMessagesTx();
    virtual unsigned long GetMessagesRx();
    
    virtual size_t Receive(zmq_msg_t *msg, const std::string& flag);
    virtual size_t Send(zmq_msg_t *msg, const std::string& flag);
    
    virtual void Bind(const std::string& address);
    virtual void Connect(const std::string& address);
    
    virtual void* GetSocket();
    virtual void Close();
     
  private:
    void* fSocket;
    std::string fId;
    unsigned long fBytesTx;
    unsigned long fBytesRx;
    unsigned long fMessagesTx;
    unsigned long fMessagesRx;
    
    int GetConstant(const std::string& constant);
};

#endif
