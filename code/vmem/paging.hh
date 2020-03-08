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
    bool dirty;
} Victim;

// Poner en -1 el proceso
void DeleteID(Victim *v, void *arg);

// para borrar todas las victimas correspondientes a un process
void DeleteVictims(SpaceId process);

// Retorna la pŕoxima víctima utilizando FIFO
Victim* GiveVictimFIFO();

// Retorna la próxima víctima
Victim *GiveVictim();

// Actualiza el puntero a la próxima víctima
void UpdatePointer();

// Retorna la página física de la próxima víctima.
int GiveVictimArray();

// Imprime un elemento de tipo Victim
void PrintVictim(Victim *v);

// Imprime la lista victims
void PrintVictims();

// Dado un puntero a thread y un buffer, carga en el buffer el nombre del archivo swap
void GiveSwapName(char *name, Thread *thread);

// Crea el archivo swap. No usada
bool CreateSwapFile(Thread *thread);

// Elimino el archivo swap. No usada
bool RemoveSwapFile(Thread *thread);

// Apaga el bit de validez de la página vpn en la TLB, si existiera
void invalidateEntry(unsigned vpn);

// Actualiza los bits dirty y use de la página virtual vpn
void updatePT(TranslationEntry *pageTable, unsigned vpn);

// Copia desde la memoeria hacia el archivo swap correspondiente.
bool toSwap(Thread *thread, unsigned vpn);

// Busca el índice de la TLB que corresponde a la página virtual vpn.
// En caso de no encontrarlo, retorna -1.
int FromVPNtoIndex(unsigned vpn);

// Apaga el i-ésimo bit de uso en table, a menos que sea < 0.
void SetUseBitOff(TranslationEntry *table, int index);

// Apaga los bits de uso en la tabla de páginas y en la TLB
void SetAllUseBitOff(Thread *thread);

//
void DiscardAccesses();
#endif
