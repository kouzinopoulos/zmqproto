#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoPublisher.h"

using namespace std;

void *zmqprotoPublisher::Run(void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  int fEventSize = 10000;
  
  Content* payload = new Content[fEventSize];
  for (int i = 0; i < fEventSize; ++i) {
        (&payload[i])->id = 0;
        (&payload[i])->x = rand() % 100 + 1;
        (&payload[i])->y = rand() % 100 + 1;
        (&payload[i])->z = rand() % 100 + 1;
        (&payload[i])->a = (rand() % 100 + 1) / (rand() % 100 + 1);
        (&payload[i])->b = (rand() % 100 + 1) / (rand() % 100 + 1);
  }
  
  int counter = 0;
  
  //Initialize publisher socket
  zmq::socket_t publisher(*context, ZMQ_PUSH);
  publisher.connect("tcp://localhost:5556");
  
  //Send messages to the proxy, 1 per second
  while (1) {
    zmq::message_t msg (fEventSize * sizeof(Content));
    memcpy(msg.data(), payload, fEventSize * sizeof(Content));
    publisher.send (msg);
    
    cout << self_id << " " << determine_ip() << " -> Sent message " << counter++ << endl;
    
    sleep(1);
  }
}

