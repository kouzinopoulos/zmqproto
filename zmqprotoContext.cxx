#include <iostream>
#include <sstream>

#include <zmq.h>

#include "zmqprotoContext.h"

using namespace std;

zmqprotoContext::zmqprotoContext(int numIoThreads)
{
  fContext = zmq_ctx_new ();
  if (fContext == NULL){
    cout << "failed creating context, reason: " << zmq_strerror(errno) << endl;
  }

  int rc = zmq_ctx_set (fContext, ZMQ_IO_THREADS, numIoThreads);
  if (rc != 0){
    cout << "failed configuring context, reason: " << zmq_strerror(errno) << endl;
  }
}

zmqprotoContext::~zmqprotoContext()
{
  Close();
}

void* zmqprotoContext::GetContext()
{
  return fContext;
}

void zmqprotoContext::Close()
{
  if (fContext == NULL){
    return;
  }

  int rc = zmq_ctx_destroy (fContext);
  if (rc != 0) {
    cout << "failed closing context, reason: " << zmq_strerror(errno) << endl;
  }

  fContext = NULL;
}
