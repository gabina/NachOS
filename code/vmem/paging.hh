#ifndef NACHOS_VMEM_PAGING__HH
#define NACHOS_VMEM_PAGING__HH

// for SpaceId
#include "../userprog/syscall.h"

// for class List
#include "../threads/list.hh"

// for class Thread
#include "../threads/thread.hh"

//la cantidad de caracteres máxima que puede tener el nombre de un archivo swap
#define MAX_NSWAP 10

typedef struct _Victim {
    SpaceId process;
    unsigned virtualPage;
} Victim;

// Poner en -1 el proceso
void DeleteID(Victim *v, void *arg);

// para borrar todas las victimas correspondientes a un process
void DeleteVictims(SpaceId process);

// Retorna la próxima víctima
Victim *GiveVictim();

// Dado un puntero a thread y un buffer, carga en el buffer el nombre del archivo swap
void GiveSwapName(char *name, Thread *thread);

// Crea el archivo swap
bool CreateSwapFile(Thread *thread);

// Elimino el archivo swap
bool RemoveSwapFile(Thread *thread);

void invalidateEntry(unsigned vpn);

void updatePT(TranslationEntry *pageTable, unsigned vpn);

bool toSwap(Thread *thread, unsigned vpn);
#endif