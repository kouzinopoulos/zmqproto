#include <string.h>
#include <stdio.h>

#include "zmqprotoCommon.h"

using namespace std;

char *zmqprotoCommon::determine_ip()
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
