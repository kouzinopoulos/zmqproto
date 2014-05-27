#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoCommon.h"
#include "zmqprotoContext.h"
#include "zmqprotoSocket.h"
#include "zmqprotoSubscriber.h"

using namespace std;

char localIPAddr[30], directoryIPAddr[30];

void Log (zmqprotoSocket *FLPSocketPtr, zmqprotoSocket *directorySocketPtr)
{
  unsigned long bytesTx = 0;
  unsigned long messagesTx = 0;
  unsigned long bytesRx = 0;
  unsigned long messagesRx = 0;
  
  while (1) {
    cout << "Tx " << "\033[01;34m" << directorySocketPtr->GetBytesTx() - bytesTx << " b/s "
         << directorySocketPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m Rx " << "\033[01;31m"
         << FLPSocketPtr->GetBytesRx() - bytesRx << " b/s "
         << FLPSocketPtr->GetMessagesRx() - messagesRx << " msg/s" << "\033[0m" << endl;
         
    bytesTx = directorySocketPtr->GetBytesTx();
    messagesTx = directorySocketPtr->GetMessagesTx();
    bytesRx = FLPSocketPtr->GetBytesRx();
    messagesRx = FLPSocketPtr->GetMessagesRx();
    
    boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}

void pushToDirectory (zmqprotoSocket *fSocketPtr)
{
  //Connect to directory
  fSocketPtr->Connect(directoryIPAddr);

  //On each time slice, send the whole "tcp://IP:port" identifier to the directory
  while (1) {
    zmq_msg_t msgToDirectory;
    zmq_msg_init_size (&msgToDirectory, 30);
    memcpy(zmq_msg_data (&msgToDirectory), localIPAddr, 30);
    
    fSocketPtr->Send (&msgToDirectory, "");
    
    //cout << "EPN: Sent a ping to the directory" << endl;
    
    boost::this_thread::sleep(boost::posix_time::seconds(10));
  }
}

void *pullFromFLP (zmqprotoSocket *fSocketPtr)
{
  //Bind locally
  fSocketPtr->Bind(localIPAddr);
  
  //Receive payload from the FLPs
  while (1) {
    zmq_msg_t msgFromFLP;
    zmq_msg_init (&msgFromFLP);
    fSocketPtr->Receive (&msgFromFLP, "");
    
    Content* input = reinterpret_cast<Content*>(zmq_msg_data (&msgFromFLP));
/*    
    cout << "EPN: Received payload " << (&input[0])->id << " from FLP" << endl;
    cout << "EPN: Message size: " << zmq_msg_size (&msgFromFLP) << " bytes" << endl;
    cout << "EPN: message content: " << (&input[0])->x << " " << (&input[0])->y << " " << (&input[0])->z << " " << (&input[0])->a << " " << (&input[0])->b << endl << endl;
*/
  }
}

int main(int argc, char** argv)
{
  if ( argc != 5 ) {
    cout << "Usage: " << argv[0] << " localIPAddr localIPPort directoryIPAddr directoryIPPort" << endl;
    return 1;
  }
  
  snprintf(localIPAddr, 30, "tcp://%s:%s", argv[1], argv[2]);
  snprintf(directoryIPAddr, 30, "tcp://%s:%s", argv[3], argv[4]);
  
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Initialize sockets
  zmqprotoSocket FLPSocket(fContext.GetContext(), "pull", 0);
  zmqprotoSocket directorySocket(fContext.GetContext(), "push", 1);
  
  //Launch the threads that handle the sockets
  boost::thread directoryThread(pushToDirectory, &directorySocket);
  boost::thread FLPThread(pullFromFLP, &FLPSocket);
  
  boost::thread logThread(Log, &FLPSocket, &directorySocket);

  directoryThread.join();
  FLPThread.join();
  
  logThread.join();
}
