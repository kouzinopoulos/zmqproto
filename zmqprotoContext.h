#ifndef ZMQPROTOCONTEXT_H
#define ZMQPROTOCONTEXT_H

class zmqprotoContext
{
  public:
    zmqprotoContext(int numIoThreads);
    virtual ~zmqprotoContext();
  
    void* GetContext();
    void Close();
  
  private:
    void* fContext;
};

#endif
