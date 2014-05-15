#ifndef ZMQPROTO_H
#define ZMQPROTO_H

#include<vector>

//Needed for strings and vectors
using namespace std;

struct Content {
  int id;
  double a;
  double b;
  int x;
  int y;
  int z;
};

class zmqproto
{
  public:
    
  private:
  
    void publisher();
    void subscriber();
};

#endif
