#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <vector>

#include <unistd.h>

#include <sys/ioctl.h> //For ioctl
#include <net/if.h>
#include <netinet/in.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoContext.h"
#include "zmqprotoSocket.h"
#include "zmqproto.h"

using namespace std;

void Log (zmqprotoSocket *frontendPtr, zmqprotoSocket *backendPtr)
{
  unsigned long bytesTx = 0;
  unsigned long messagesTx = 0;
  unsigned long bytesRx = 0;
  unsigned long messagesRx = 0;
  
  while (1) {
    cout << "Tx " << "\033[01;34m" << frontendPtr->GetBytesTx() - bytesTx << " b/s "
         << frontendPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m Rx " << "\033[01;31m"
         << backendPtr->GetBytesRx() - bytesRx << " b/s "
         << backendPtr->GetMessagesRx() - messagesRx << " msg/s" << "\033[0m" << endl;
         
    bytesTx = frontendPtr->GetBytesTx();
    messagesTx = frontendPtr->GetMessagesTx();
    bytesRx = backendPtr->GetBytesRx();
    messagesRx = backendPtr->GetMessagesRx();
    
    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}

int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    cout << "Usage: " << argv[0] << " frontendIPPort backendIPPort" << endl;
    return 1;
  }
  
  if ( atoi(argv[1]) > 65535 || atoi(argv[2]) > 65535 || atoi(argv[1]) < 1 || atoi(argv[2]) < 1 ) {
    cout << "Usage: " << argv[0] << " frontendIPPort backendIPPort" << endl;
    return 1;
  }
  
  char frontendIPAddr[30], backendIPAddr[30];
  snprintf(frontendIPAddr, 30, "tcp://*:%s", argv[1]);
  snprintf(backendIPAddr, 30, "tcp://*:%s", argv[2]);
  
  //Initialize the IP vector
  vector<string>ipVector;
  vector<string>::iterator ipVectorIter;
  
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Setup the directory sockets
  zmqprotoSocket frontend(fContext.GetContext(), "push", 0);
  frontend.Bind(frontendIPAddr);

  zmqprotoSocket backend(fContext.GetContext(), "pull", 1);
  backend.Bind(backendIPAddr);

  boost::thread logThread(Log, &frontend, &backend);
  
  //cout << "Directory: Waiting for incoming connections..." << endl;
  
  while (1) {
    zmq_msg_t subMsg;
    zmq_msg_init (&subMsg);
    backend.Receive(&subMsg, "");
    
    //cout << "Directory: Received a ping from EPN" << endl;
    
    //If the IP is unknown, add it to the IP vector
    ipVectorIter = find (ipVector.begin(), ipVector.end(), reinterpret_cast<char *>(zmq_msg_data (&subMsg)));
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      ipVector.push_back(reinterpret_cast<char *>(zmq_msg_data (&subMsg)));
      
      //cout << "Directory: Unknown IP, adding it to vector. Total IPs: " << ipVector.size() << endl;
    }

    //Pack the IP vector using msgpack and send it to all connected FLPs
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, ipVector);

    //zmq::message_t pubMsg(sbuf.size());
    zmq_msg_t pubMsg;
    zmq_msg_init_size (&pubMsg, sbuf.size());
    memcpy(zmq_msg_data (&pubMsg), sbuf.data(), sbuf.size());

    frontend.Send(&pubMsg, "");
    
    //cout << "Directory: Sent the IP vector to all FLPs" << endl << endl;
  }
  
  logThread.join();
  
  return 0;
}
