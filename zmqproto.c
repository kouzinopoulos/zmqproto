//0MQprototype
//Features so far: 
//1) FLPs and EPNs are multithreaded
//2) Using a message broker, updated each second, that keeps track of the connected FLPs and EPNs

#include "zhelpers.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MSG_SIZE 6
#define MAX_FLP 1000
#define MAX_EPN 1000

//FIXME: Open a second socket to directly communicate with the EPNs
void *FLP (void *context)
{
  //Initialize publisher
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
}

//FIXME: Open a second socket to directly communicate with the FLPs
void *EPN (void *context)
{
  //Initialize publisher
  void *socket = zmq_socket (context, ZMQ_PUSH);
  int rc = zmq_connect (socket, "tcp://localhost:5558");
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
}

int compare(int *x, int *y) 
{   
   return (*x - *y); 
}

//FIXME: store ip addresses on a (binary) list
int main(int argc, char** argv)
{
  if ( argc != 3 ) {
    printf("Usage: 0mqproto\tnumFLP\tnumEPN\n");
    return 1;
  }
  
  int numFLP = atoi(argv[1]);
  int numEPN = atoi(argv[2]);
  
  if ( numFLP <= 0 || numEPN <= 0 ) {
   printf("Error: At least 1 FLP and EPN needed\n");
   return -1;
  }
  
  if ( numFLP > MAX_FLP || numEPN > MAX_EPN ) {
   printf("Error: Too many FLPs or EPNs\n");
   return -1;
  }
  
  //Initialize zmq
  void *context = zmq_ctx_new ();

  int i;
  
  //Start publisher on a new thread
  for ( i = 0; i < numFLP; i++ ) {
    pthread_t publisherThread;
    pthread_create (&publisherThread, NULL, FLP, context);
  }
  
  //Start subscriber on a new thread
  for ( i = 0; i < numEPN; i++ ) {
    pthread_t subscriberThread;
    pthread_create (&subscriberThread, NULL, EPN, context);
  }
  
  //Create incoming sockets
  void *frontend = zmq_socket (context, ZMQ_PULL);
  int rc = zmq_bind (frontend, "tcp://*:5556");
  assert (rc == 0);

  void *backend = zmq_socket (context, ZMQ_PULL);
  rc = zmq_bind (backend, "tcp://*:5558");
  assert (rc == 0);
  
  char host [NI_MAXHOST];
  
  struct sockaddr_storage ss;
  socklen_t addrlen = sizeof ss;
  
  int srcFd;
  
  zmq_msg_t msg;
  
  //Initialize two IP arrays, one for FLPs and one for EPNs
  char *iparrayFLP[MAX_FLP] = { 0 };
  char *iparrayEPN[MAX_EPN] = { 0 };
  
  //Number of addresses on the list of FLPs and EPNs
  size_t elemsFLP = 0;
  size_t elemsEPN = 0;
  
  //Wait on messages from both sockets,
  //FIXME: Poll the incoming messages from the sockets
  while (1) {
    //Frontend
    rc = zmq_msg_init(&msg);
    assert (rc == 0);
  
    zmq_recvmsg(frontend, &msg, 0);
    
    srcFd = zmq_msg_get(&msg, ZMQ_SRCFD);
    assert(srcFd >= 0);
          
    // get the remote endpoint
    rc = getpeername (srcFd, (struct sockaddr*) &ss, &addrlen);
    assert (rc == 0);
    
    rc = getnameinfo ((struct sockaddr*) &ss, addrlen, host, sizeof host, NULL, 0, NI_NUMERICHOST);
    assert (rc == 0);
    
    printf("Received message from host %s\n", host);

    //If the IP address is not found, it will be appended to the list using lsearch
    char *FLPIPAddress = host;
    
    lsearch (&FLPIPAddress, &iparrayFLP, &elemsFLP, sizeof (int), (int(*) (const void *, const void *)) compare);
    
    //Backend
    rc = zmq_msg_init(&msg);
    assert (rc == 0);
  
    zmq_recvmsg(backend, &msg, 0);
    
    srcFd = zmq_msg_get(&msg, ZMQ_SRCFD);
    assert(srcFd >= 0);
          
    // get the remote endpoint

    rc = getpeername (srcFd, (struct sockaddr*) &ss, &addrlen);
    assert (rc == 0);
    
    rc = getnameinfo ((struct sockaddr*) &ss, addrlen, host, sizeof host, NULL, 0, NI_NUMERICHOST);
    assert (rc == 0);
    
    printf("Received message from host %s\n", host);
    
    //If the IP address is not found, it will be appended to the list using lsearch
    char *EPNIPAddress = host;
    
    lsearch (&EPNIPAddress, &iparrayEPN, &elemsEPN, sizeof (int), (int(*) (const void *, const void *)) compare);
    
    printf("Number of connected nodes: FLPs: %i EPNs: %i\n", elemsFLP, elemsEPN);
  }
  
  zmq_close (frontend);
  zmq_close (backend);
  zmq_ctx_destroy (context);
  
  int j = 0, k = 0;
  while (j < sizeof(iparrayFLP))
  {
    free(iparrayFLP[j]);
  }
  
  while (k < sizeof(iparrayEPN))
  {
    free(iparrayEPN[k]);
  }
  
  return 0;
}

