#ifndef TELE_SHARE_H
#define TELE_SHARE_H
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/shm.h>
#define MAX_INDEX (10000 / 32 + 1)
#define ONE_DAY_MINUTE (24 *60)
#define TELE_KEY 3833873
namespace tele {
struct TeleMem {
  int fetch_map[ONE_DAY_MINUTE][MAX_INDEX];
};
#define SET_BIT_INDEX(s, m, i) \
  s->fetch_map[m][i >> 5] |= (1 << (i & 0x1f))
#define IS_SET(s, m, i) \
  s->fetch_map[m][i >> 5] & (1 << (i &0x1f))
#define CLEAR_SET(s, m, i) \
  s->fetch_map[m][i >> 5] &= ~(1 << (i & 0x1f))

TeleMem* TeleShmLoad(); 
TeleMem* GetShareMem(); 
}
#endif
