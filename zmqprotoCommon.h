#ifndef ZMQPROTOCOMMON_H
#define ZMQPROTOCOMMON_H

using namespace std;

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
    static char *determine_ip();
};

#endif
