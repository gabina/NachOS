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

// Poner en -1 el proceso
void DeleteID(Victim *v, void *arg);

// para borrar todas las victimas correspondientes a un process
void DeleteVictims(List<Victim*> victims, SpaceId process);

// Retorna la próxima víctica
Victim GiveVictim(List<Victim*> victims);

#endif