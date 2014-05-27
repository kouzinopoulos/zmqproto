#include <iostream>
#include <sstream>

#include "zmqprotoContext.h"
#include "zmqprotoSocket.h"

using namespace std;

zmqprotoSocket::zmqprotoSocket (void* fContext, const string& type, int num) :
  fBytesTx(0),
  fBytesRx(0),
  fMessagesTx(0),
  fMessagesRx(0)
{
  stringstream id;
  id << type << "." << num;
  fId = id.str();

  fSocket = zmq_socket(fContext, GetConstant(type));

  int rc = zmq_setsockopt(fSocket, ZMQ_IDENTITY, &fId, fId.length());
  if (rc != 0) {
    cout << "failed setting socket option, reason: " << zmq_strerror(errno) << endl;
  }
  
  if (type == "sub") {
    rc = zmq_setsockopt(fSocket, ZMQ_SUBSCRIBE, NULL, 0);
    if (rc != 0) {
      cout << "failed setting socket option, reason: " << zmq_strerror(errno) << endl;
    }
  }

  cout << "created socket #" << fId << endl;
}

unsigned long zmqprotoSocket::GetBytesTx()
{
  return fBytesTx;
}

unsigned long zmqprotoSocket::GetBytesRx()
{
  return fBytesRx;
}

unsigned long zmqprotoSocket::GetMessagesTx()
{
  return fMessagesTx;
}

unsigned long zmqprotoSocket::GetMessagesRx()
{
  return fMessagesRx;
}

void zmqprotoSocket::Close()
{
  if (fSocket == NULL){
    return;
  }

  int rc = zmq_close (fSocket);
  if (rc != 0) {
    cout << "failed closing socket, reason: " << zmq_strerror(errno) << endl;
  }

  fSocket = NULL;
}

void* zmqprotoSocket::GetSocket()
{
  return fSocket;
}

size_t zmqprotoSocket::Send(zmq_msg_t *msg, const string& flag)
{
  int nbytes = zmq_msg_send (msg, fSocket, GetConstant(flag));
  if (nbytes >= 0){
    fBytesTx += nbytes;
    ++fMessagesTx;
    return nbytes;
  }
  if (zmq_errno() == EAGAIN){
    return false;
  }
  cout << "failed sending on socket #" << fId << ", reason: " << zmq_strerror(errno) << endl;
  return nbytes;
  
}

size_t zmqprotoSocket::Receive(zmq_msg_t *msg, const string& flag)
{
  int nbytes = zmq_msg_recv (msg, fSocket, GetConstant(flag));
  if (nbytes >= 0){
    fBytesRx += nbytes;
    ++fMessagesRx;
    return nbytes;
  }
  if (zmq_errno() == EAGAIN){
    return false;
  }
  cout << "failed receiving on socket #" << fId << ", reason: " << zmq_strerror(errno) << endl;
  return nbytes;
}

void zmqprotoSocket::Bind(const string& address)
{
  cout << "bind socket " << fId << " on " << address << endl;

  int rc = zmq_bind (fSocket, address.c_str());
  if (rc != 0) {
    cout << "failed binding socket #" << fId << ", reason: " << zmq_strerror(errno) << endl;
  }
}

void zmqprotoSocket::Connect(const string& address)
{
  cout << "connect socket #" << fId << " on " << address << endl;

  int rc = zmq_connect (fSocket, address.c_str());
  if (rc != 0) {
    cout << "failed connecting socket #" << fId << ", reason: " << zmq_strerror(errno) << endl;
  }
}

int zmqprotoSocket::GetConstant(const string& constant)
{
  if (constant == "") return 0;
  if (constant == "sub") return ZMQ_SUB;
  if (constant == "pub") return ZMQ_PUB;
  if (constant == "xsub") return ZMQ_XSUB;
  if (constant == "xpub") return ZMQ_XPUB;
  if (constant == "push") return ZMQ_PUSH;
  if (constant == "pull") return ZMQ_PULL;
  if (constant == "snd-hwm") return ZMQ_SNDHWM;
  if (constant == "rcv-hwm") return ZMQ_RCVHWM;
  if (constant == "snd-more") return ZMQ_SNDMORE;
  if (constant == "rcv-more") return ZMQ_RCVMORE;
  if (constant == "no-block") return ZMQ_NOBLOCK;
  return -1;
}
