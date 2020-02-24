#ifndef NACHOS_VMEM_PAGING__HH
#define NACHOS_VMEM_PAGING__HH

// for SpaceId
#include "../userprog/syscall.h"

// for class List
#include "../threads/list.hh"

typedef struct _Victim {
    SpaceId process;
    unsigned virtualPage;
} Victim;

// para borrar todas las victimas correspondientes a un process
void deleteVictims(List<Victim> victims, SpaceId proces) {

}


#endif