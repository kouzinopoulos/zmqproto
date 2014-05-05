#include <zmq.hpp>

#include <iostream>
#include <unistd.h>

#include <sys/ioctl.h> //For ioctl
#include <net/if.h>
#include <netinet/in.h>

#include "0mqproto.h"

//Needed for strings
using namespace std;
/*
int utilities_get_my_ip (const char * device)
{
  int fd;
  struct ifreq ifr;

  fd = socket (AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET; 
  strncpy (ifr.ifr_name, device, IFNAMSIZ-1);

  ioctl (fd, SIOCGIFADDR, &ifr);

  close (fd);
  return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}
*/
char *determine_ip()
{
  FILE *fp = popen("ifconfig", "r");
  
  if (fp) {
    char *p=NULL, *e;
    size_t n;
    while ((getline(&p, &n, fp) > 0) && p) {
      if (p = strstr(p, "inet ")) {
        p+=5;
        if (p = strchr(p, ':')) {
          ++p;
          if (e = strchr(p, ' ')) {
            *e='\0';
          }
          //Break at the first occurrence of a non-localhost ip
          if (! strstr(p, "127.0.0.1")) {
              break;
          }
        }
      }
    }
    pclose(fp);
    return p;
  }
  else {
    return NULL;
  }
}

void *publisher (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  //Initialize publisher
  zmq::socket_t publisher(*context, ZMQ_PUSH);
  publisher.connect("tcp://localhost:5556");
  
  //Send messages to the proxy, 1 per second
  while (1) {
    zmq::message_t publisherMessage (5);
    memcpy ((void *) publisherMessage.data(), "World", 5);
    publisher.send (publisherMessage);
    
    sleep(1);
  }
}

void *subscriber (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  //Initialize subscriber
  zmq::socket_t subscriber(*context, ZMQ_PULL);
  subscriber.connect("tcp://localhost:5558");
  
  //Receive messages from the subscriber
  while (1) {
    zmq::message_t subscriberMessage;
    subscriber.recv (&subscriberMessage);
  
    printf("%u: Received message\n", self_id);
  }
}
/*
void *proxy (void *arg)
{
  zmq::context_t *context = (zmq::context_t *) arg;
  
  pthread_t self_id = pthread_self();
  
  //Initialize proxy
  zmq::socket_t frontend (*context, ZMQ_PULL);
  frontend.bind("tcp://*:5556");

  zmq::socket_t backend (*context, ZMQ_PUSH);
  backend.bind("tcp://*:5558");
  
  zmq_proxy (frontend, backend, NULL);
  
}
*/

int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    cout << "Usage: 0mqproto\tnumFLP\tnumEPN" << endl;
    return 1;
  }

  cout<<determine_ip()<<endl;
  
  int numFLP = atoi(argv[1]);
  int numEPN = atoi(argv[2]);
  
  if ( numFLP <= 0 || numEPN <= 0 ) {
    cout << "Error: At least 1 FLP and EPN needed\n" << endl;
    return -1;
  }
  
  //Initialize zmq
  zmq::context_t context (1);

  // Start proxy
  // pthread_t proxyThread;
  // pthread_create (&proxyThread, NULL, proxy, &context);
    
  //Start publisher on a new thread
  for ( int i = 0; i < numFLP; i++ ) {
    pthread_t publisherThread;
    pthread_create (&publisherThread, NULL, publisher, &context);
  }
  
  //Start subscriber on a new thread
  for ( int i = 0; i < numEPN; i++ ) {
    pthread_t subscriberThread;
    pthread_create (&subscriberThread, NULL, subscriber, &context);
  }
  
  //Block on proxy
  zmq::socket_t frontend (context, ZMQ_PULL);
  frontend.bind("tcp://*:5556");

  zmq::socket_t backend (context, ZMQ_PUSH);
  backend.bind("tcp://*:5558");
  
  zmq_proxy (frontend, backend, NULL);
  
  return 0;
}

