#ifndef ZMQPROTOCOMMON_H
#define ZMQPROTOCOMMON_H

#include <sstream>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <ctime>

using std::ostringstream;

struct Content {
  int id;
  double a;
  double b;
  int x;
  int y;
  int z;
};

class zmqprotoCommon
{
  public:
    enum {
      DEBUG, INFO, ERROR, STATE
    };
    virtual ~zmqprotoCommon();
    ostringstream& Print();
  
  private:
    ostringstream os;
};

#define PRINT zmqprotoCommon().Print()

#endif
