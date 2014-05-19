#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <unistd.h>

#include "zmqprotoCommon.h"
#include "zmqprotoPublisher.h"

using namespace std;

int main(int argc, char** argv)
{
  //Initialize zmq
  zmq::context_t context (1);
  
  //Initialize socket to pull from the directory
  zmq::socket_t pullFromDirectory(context, ZMQ_PULL);
  pullFromDirectory.connect("tcp://127.0.0.1:5556");
  
  //Initialize socket to push to the EPN
  zmq::socket_t pushToEPN(context, ZMQ_PUSH);
  
  //Initialize dummy data
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
  
  char ipAddr[30];
  
  while (1) {
    zmq::message_t msg;
    pullFromDirectory.recv(&msg);
    
    // Deserialize received message
    msgpack::unpacked unpacked;
    msgpack::unpack(&unpacked, reinterpret_cast<char*>(msg.data()), msg.size());
    msgpack::object obj = unpacked.get();

    std::vector<string> data;
    obj.convert(&data);
    
    cout << "FLP: Received a vector with " << data.size() << " IPs from the directory node" << endl;
    
    //Connect to each received IP, send payload
    for (int i = 0; i < data.size(); i++) {
      snprintf(ipAddr, 30, "tcp://%s:5560", data.at(i).c_str());
      pushToEPN.connect(ipAddr);
      
      zmq::message_t msgToEPN (fEventSize * sizeof(Content));
      memcpy(msgToEPN.data(), payload, fEventSize * sizeof(Content));
      pushToEPN.send (msg);
      
      cout << "FLP: Sent a message to EPN at " << data.at(i).c_str() << endl;
    }

    sleep(1);
  }
  return 0;
}
