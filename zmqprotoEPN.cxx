#include <zmq.hpp>

#include <iostream>

#include <time.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "zmqprotoCommon.h"
#include "zmqprotoContext.h"
#include "zmqprotoEPN.h"
#include "zmqprotoSocket.h"

using namespace std;

char localIPAddr[30], directoryIPAddr[30];

void Log (zmqprotoSocket *FLPSocketPtr, zmqprotoSocket *directorySocketPtr)
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
    
    cout << setprecision(0) << fixed
         << "Tx \033[01;34m" << (directorySocketPtr->GetBytesTx() - bytesTx)/1048576 << " Mb/s "
         << directorySocketPtr->GetMessagesTx() - messagesTx << " msg/s \033[0m " << "Rx \033[01;31m"
         << (FLPSocketPtr->GetBytesRx() - bytesRx)/1048576 << " Mb/s "
         << FLPSocketPtr->GetMessagesRx() - messagesRx << " msg/s \033[0m "
         << "Avg Tx \033[01;34m" << directorySocketPtr->GetBytesTx()/(difTime * 1048576) << " Mb/s "
         << directorySocketPtr->GetMessagesTx()/difTime << " msg/s \033[0m Rx "
         << "\033[01;31m" << FLPSocketPtr->GetBytesRx()/(difTime * 1048576) << " Mb/s "
         << FLPSocketPtr->GetMessagesRx()/difTime << " msg/s \033[0m " << '\r' << flush;
         
    bytesTx = directorySocketPtr->GetBytesTx();
    messagesTx = directorySocketPtr->GetMessagesTx();
    bytesRx = FLPSocketPtr->GetBytesRx();
    messagesRx = FLPSocketPtr->GetMessagesRx();
    
    boost::this_thread::sleep (boost::posix_time::seconds(1));
  }
}

void pushToDirectory (zmqprotoSocket *fSocketPtr)
{
  //Connect to directory
  fSocketPtr->Connect (directoryIPAddr);

  //On each time slice, send the whole "tcp://IP:port" identifier to the directory
  while (1) {
    zmqprotoMessage msg (30);
    memcpy (msg.GetData(), localIPAddr, 30);
    
    fSocketPtr->Send (&msg, "");
    
    PRINT << "Sent a ping to the directory";
    
    boost::this_thread::sleep (boost::posix_time::seconds(10));
  }
}

void pullFromFLP (zmqprotoSocket *fSocketPtr)
{
  //Bind locally
  fSocketPtr->Bind (localIPAddr);
  
  //Receive payload from the FLPs
  while (1) {
    zmqprotoMessage msg;
    fSocketPtr->Receive (&msg, "");
    
    Content* input = reinterpret_cast<Content*>(msg.GetData());
    
    PRINT << "Received payload " << (&input[0])->id << " from FLP. Message size: "
          << msg.GetSize() << " bytes.";
  }
}

int main(int argc, char** argv)
{
  if (argc != 5) {
    cout << "Usage: " << argv[0] << " localIPAddr localIPPort directoryIPAddr directoryIPPort" << endl;
    return 1;
  }
  
  if (atoi (argv[2]) > 65535 || atoi (argv[4]) > 65535 || atoi (argv[2]) < 1 || atoi (argv[4]) < 1) {
    cout << "Usage: " << argv[0] << " localIPAddr localIPPort directoryIPAddr directoryIPPort" << endl;
    return 1;
  }
  
  snprintf (localIPAddr, 30, "tcp://%s:%s", argv[1], argv[2]);
  snprintf (directoryIPAddr, 30, "tcp://%s:%s", argv[3], argv[4]);
  
  //Initialize zmq
  int numIoThreads = 1;
  zmqprotoContext fContext (numIoThreads);
  
  //Initialize sockets
  zmqprotoSocket FLPSocket (fContext.GetContext(), "pull", 0);
  zmqprotoSocket directorySocket (fContext.GetContext(), "push", 1);
  
  //Launch the threads that handle the sockets
  boost::thread directoryThread (pushToDirectory, &directorySocket);
  boost::thread FLPThread (pullFromFLP, &FLPSocket);

#ifndef DEBUGMSG
  boost::thread logThread (Log, &FLPSocket, &directorySocket);
#endif

  //Wait for the threads to finish execution
  directoryThread.join();
  FLPThread.join();

#ifndef DEBUGMSG
  logThread.join();
#endif
}
