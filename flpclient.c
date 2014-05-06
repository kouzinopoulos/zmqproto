//0MQprototype
//Features so far: 
//1) FLPs and EPNs are multithreaded
//2) Using a message broker, updated each second, that keeps track of the connected FLPs and EPNs

#include "zhelpers.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MSG_SIZE 6

//FIXME: Open a second socket to directly communicate with the EPNs
int main(int argc, char** argv)
{
  //Initialize zmq
  void *context = zmq_ctx_new ();

  //Initialize client
  void *socket = zmq_socket (context, ZMQ_PUSH);
  int rc = zmq_connect (socket, "tcp://localhost:5556");
  assert (rc == 0);

  //Send messages to the message broker, 1 per second
  while (1) {
    char tmp[MSG_SIZE];
    sprintf (tmp, "Hello");
    zmq_send(socket, tmp, MSG_SIZE, 0);
    
    sleep(1);
  }
  
  zmq_close (socket);
  zmq_ctx_destroy (context);
  
  return 0;
}

