#include "paging.hh"
#include "../threads/system.hh"

// Función a aplicar a cada elemento de la lista de víctimas
void DeleteID(Victim *v, void *arg)
{
    if (v->process == *((SpaceId *)arg))
        v->process = -1;
}

// Borra todas las victimas correspondientes a un process
void DeleteVictims(SpaceId process) 
{
    victims->Apply2(DeleteID,(void*) &process);
}

// Devuelve la primera víctima con un spaceID válido
Victim* GiveVictim()
{
	bool victimFound = false;
    Victim *v;
	List<Victim*> offVictims;
    while(!victims->IsEmpty()){
		v = victims->Remove();
		if(v->process == -1)
			continue;
		Thread *thread = processTable->GetProcess(v->process);
		TranslationEntry *pT = (thread->space)->pageTable;
		/* Si el bit está prendido, lo apago y agrego el elemento nuevamente*/
		if(pT[v->virtualPage].use){
			//printf("VPN %u prendida\n",v->virtualPage);
			SetUseBitOff(pT,v->virtualPage);
			if( currentThread == thread)
				SetUseBitOff(machine->tlb,FromVPNtoIndex(v->virtualPage));
			// Los nodos apagados los acumulo en una lista
			offVictims.Prepend(v);
		}else{
			victimFound = true;
			break;
		}
	}

	// Agrego los nodos ahora apagados
	while(!offVictims.IsEmpty())
		victims->Prepend(offVictims.Remove());

	if(!victimFound)
		return GiveVictim();		
	return v;
}

void PrintVictim(Victim *v)
{
	printf("Proceso: %d VPN: %u| ",v->process,v->virtualPage);
}

void PrintVictims()
{
	victims->Apply(PrintVictim);
	printf("\n");
}
// Dado un puntero a thread y un buffer, carga en el buffer el nombre del archivo swap
void GiveSwapName(char *name, Thread *thread)
{
    snprintf(name, MAX_NSWAP, "SWAP._%d", (int) (thread->GetID()));
}

bool CreateSwapFile(Thread *thread)
{
    char* fileName = new char [MAX_NSWAP];
    GiveSwapName(fileName,thread);
    bool ret = fileSystem->Create(fileName, 0);
    delete fileName;
    return ret;
}

bool RemoveSwapFile(Thread *thread)
{
    char* fileName = new char [MAX_NSWAP];
    GiveSwapName(fileName,thread);
    bool ret = fileSystem->Remove(fileName);
    delete fileName;
    return ret;
}

/* Invalida la entrada de la TLB correspondiente a la página virtual vpn
 * Si no existe la entrada, no hace nada*/
void invalidateEntry(unsigned vpn)
{
	for(unsigned i = 0; i < TLB_SIZE; i++)
    	if(machine->tlb[i].valid && (machine->tlb[i].virtualPage == vpn)){
			machine->tlb[i].valid = false;
			break;
		}
		
}

/* Actualiza la tabla de páginas respecto a la entrada en la TLB*/
void updatePT(TranslationEntry *pageTable, unsigned vpn)
{
	int i = FromVPNtoIndex(vpn);
	if(i!=-1){
		pageTable[vpn].dirty = machine->tlb[i].dirty;
		pageTable[vpn].use = machine->tlb[i].use;
	}
}

/* Escribe en el archivo SWAP correspondiente la página virtual vpn
 * perteneciente al hilo thread
 * Prende el bit swap
 * Apaga el bit de validez en la page table (la página ya no está en memoria)
 * Libera la memoria*/
bool toSwap(Thread *thread, unsigned vpn)
{
	TranslationEntry *pT = (thread->space)->pageTable;
	unsigned physicalPage = pT[vpn].physicalPage;
	int block = PAGE_SIZE * vpn;
	int written = PAGE_SIZE;
	/* Si nunca se hizo swap o si la página está sucia, debo copiar la página al swap */
	if(!pT[vpn].swap || pT[vpn].dirty)
		written = (thread->swap)->WriteAt(&(machine->mainMemory[physicalPage*PAGE_SIZE]),
											PAGE_SIZE,block);
	if (written != PAGE_SIZE)
		return false;
	else {
		// Indico que la página está en SWAP
		pT[vpn].swap = true;
		// Indico que la página no está en memoria
		pT[vpn].valid = false;
		// Libero la página física
		bitmap->Clear(physicalPage);
		return true;
	}
}

int FromVPNtoIndex(unsigned vpn)
{
	for(unsigned i = 0; i < TLB_SIZE; i++)
      	if(machine->tlb[i].valid && (machine->tlb[i].virtualPage == vpn))
			return i;
	return -1;	
}

void SetUseBitOff(TranslationEntry *table, int index)
{
	if(index >= 0)
		table[index].use = false;
}


void SetAllUseBitOff(Thread *thread)
{
	TranslationEntry *pT = (thread->space)->pageTable;
	for (unsigned i = 0; i < (thread->space)->GetNumPages(); i++)
		SetUseBitOff(pT, i);
	for (unsigned i = 0; i < TLB_SIZE; i++)
		SetUseBitOff(machine->tlb, i);
}