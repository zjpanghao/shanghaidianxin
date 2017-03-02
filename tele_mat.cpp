#include "tele_mat.h"
namespace tele {
TeleMem *sharemem_g = NULL;
TeleMem *GetShareMem() {
  return sharemem_g;
}

TeleMem* TeleShmLoad() {
  int id = shmget((key_t)TELE_KEY, sizeof(TeleMem), 0666|IPC_CREAT);
  if (id == -1) {
    printf("create error %d\n", id);
    return NULL;
  }
  void *mem = shmat(id, 0, 0);
  sharemem_g = (TeleMem*)mem;
  return (TeleMem*)mem; 
}
}
