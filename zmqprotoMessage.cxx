#include <cstring>
#include <cstdlib>

#include "zmqprotoCommon.h"
#include "zmqprotoMessage.h"

zmqprotoMessage::zmqprotoMessage()
{
  int rc = zmq_msg_init (&fMessage);
  if (rc != 0) {
    PRINT << "Failed initializing message, reason: " << zmq_strerror(errno);
  }
}

zmqprotoMessage::zmqprotoMessage(size_t size)
{
  int rc = zmq_msg_init_size (&fMessage, size);
  if (rc != 0) {
    PRINT << "Failed initializing message with size, reason: " << zmq_strerror(errno);
  }
}

zmqprotoMessage::zmqprotoMessage(void* data, size_t size)
{
  int rc = zmq_msg_init_data (&fMessage, data, size, &CleanUp, NULL);
  if (rc != 0) {
    PRINT << "Failed initializing message with data, reason: " << zmq_strerror(errno);
  }
}

void* zmqprotoMessage::GetMessage()
{
  return &fMessage;
}

void* zmqprotoMessage::GetData()
{
  return zmq_msg_data (&fMessage);
}

size_t zmqprotoMessage::GetSize()
{
  return zmq_msg_size (&fMessage);
}

inline void zmqprotoMessage::CloseMessage()
{
  int rc = zmq_msg_close (&fMessage);
  if (rc != 0) {
    PRINT << "Failed closing message, reason: " << zmq_strerror(errno);
  }
}

void zmqprotoMessage::CleanUp(void* data, void* hint)
{
  free (data);
}

zmqprotoMessage::~zmqprotoMessage()
{
  int rc = zmq_msg_close (&fMessage);
  if (rc != 0) {
    PRINT << "Failed closing message with data, reason: " << zmq_strerror(errno);
  }
}
