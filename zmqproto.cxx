#include <zmq.hpp>
#include <msgpack.hpp>

#include <iostream>
#include <vector>

#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoContext.h"
#include "zmqprotoSocket.h"
#include "zmqproto.h"

using namespace std;

vector<string>ipVector;
vector<string>::iterator ipVectorIter;

void Log (zmqprotoSocket *frontendPtr, zmqprotoSocket *backendPtr)
{
  unsigned long bytesTx = 0;
  unsigned long messagesTx = 0;
  unsigned long bytesRx = 0;
  unsigned long messagesRx = 0;
  
  time_t start, current;
  time (&start);
  
  while (1) {
    time (&current);
    double difTime = difftime (current, start);
    
    cout << setprecision(2) << fixed
         << "Tx \033[01;34m" << frontendPtr->GetBytesTx() - bytesTx << " b/s "
         << frontendPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m " << "Rx \033[01;31m"
         << backendPtr->GetBytesRx() - bytesRx << " b/s "
         << backendPtr->GetMessagesRx() - messagesRx << " msg/s \033[0m "
         << "Avg Tx \033[01;34m" << frontendPtr->GetBytesTx()/difTime << " b/s "
         << frontendPtr->GetMessagesTx()/difTime << " msg/s \033[0m Rx " 
         << "\033[01;31m" << backendPtr->GetBytesRx()/difTime << " b/s "
         << backendPtr->GetMessagesRx()/difTime << " msg/s \033[0m " << '\r' << flush;
         
    bytesTx = frontendPtr->GetBytesTx();
    messagesTx = frontendPtr->GetMessagesTx();
    bytesRx = backendPtr->GetBytesRx();
    messagesRx = backendPtr->GetMessagesRx();
    
    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}

void sendToFLP (zmqprotoSocket *fSocketPtr)
{
  while (1) {
    if ( ipVector.size() > 0 ) {
      //Pack the IP vector using msgpack and send it to all connected FLPs
      msgpack::sbuffer sbuf;
      msgpack::pack(sbuf, ipVector);

      zmq_msg_t msg;
      zmq_msg_init_size (&msg, sbuf.size());
      memcpy(zmq_msg_data (&msg), sbuf.data(), sbuf.size());

      fSocketPtr->Send(&msg, "");
      
      zmq_msg_close (&msg);
      //cout << "Directory: Sent the IP vector to all FLPs" << endl << endl;
    }
    boost::this_thread::sleep(boost::posix_time::seconds(10));
  }
}

void receiveFromEPN (zmqprotoSocket *fSocketPtr)
{
  while (1) {
    zmq_msg_t msg;
    zmq_msg_init (&msg);
    fSocketPtr->Receive(&msg, "");
    
    //If the IP is unknown, add it to the IP vector
    ipVectorIter = find (ipVector.begin(), ipVector.end(), reinterpret_cast<char *>(zmq_msg_data (&msg)));
    
    if ( ipVectorIter == ipVector.end() ) {
      //FIXME: performance wise is better to allocate big blocks of memory for the vector instead of element-by-element with each push_back()
      ipVector.push_back(reinterpret_cast<char *>(zmq_msg_data (&msg)));
      
      //cout << "Directory: Unknown IP, adding it to vector. Total IPs: " << ipVector.size() << endl;
    }
    zmq_msg_close (&msg);
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
  
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Setup the directory sockets
  zmqprotoSocket frontend(fContext.GetContext(), "push", 0);
  frontend.Bind(frontendIPAddr);

  zmqprotoSocket backend(fContext.GetContext(), "pull", 1);
  backend.Bind(backendIPAddr);
  
  //Launch the threads that handle the sockets
  boost::thread FLPThread(sendToFLP, &frontend);
  boost::thread EPNThread(receiveFromEPN, &backend);

  boost::thread logThread(Log, &frontend, &backend);

  FLPThread.join();
  EPNThread.join();
  
  logThread.join();
  
  return 0;
}
