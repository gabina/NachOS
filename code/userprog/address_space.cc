/// Routines to manage address spaces (executing user programs).
///
/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "threads/system.hh"


/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
static void
SwapHeader(NoffHeader *noffH)
{
    noffH->noffMagic              = WordToHost(noffH->noffMagic);
    noffH->code.size              = WordToHost(noffH->code.size);
    noffH->code.virtualAddr       = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr        = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size          = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr   = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr    = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size        = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr  = WordToHost(noffH->uninitData.inFileAddr);
}

/// Create an address space to run a user program.
///
/// Load the program from a file `executable`, and set everything up so that
/// we can start executing user instructions.
///
/// Assumes that the object code file is in NOFF format.
///
/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
///
/// * `executable` is the file containing the object code to load into
///   memory.
AddressSpace::AddressSpace(OpenFile *executable)
{
  exec = new OpenFile(executable->file);
  unsigned   sizeData, sizeCode, lastPageBytes, size, sizeZero, numPagesZero;

  exec->ReadAt((char *) &noffH, sizeof noffH, 0);
  if (noffH.noffMagic != NOFFMAGIC &&
        WordToHost(noffH.noffMagic) == NOFFMAGIC)
      SwapHeader(&noffH);
  ASSERT(noffH.noffMagic == NOFFMAGIC);

  // How big is address space?

	// Calculo el número de páginas para el segmento de código
	numPagesCode = divRoundUp(noffH.code.size, PAGE_SIZE);
	sizeCode = numPagesCode * PAGE_SIZE;
	lastPageBytes = noffH.code.size % PAGE_SIZE;
	if (lastPageBytes == 0)
		lastPageBytes = PAGE_SIZE;

	// Calculo el número de páginas para datos inicializados
  numPagesData = divRoundUp(noffH.initData.size, PAGE_SIZE);
  sizeData = numPagesData * PAGE_SIZE;
	
  // Calculo el número de páginas que irán en cero (datos no incializados y stack)
  sizeZero = noffH.uninitData.size + USER_STACK_SIZE; 
  // We need to increase the size to leave room for the stack.
  numPagesZero = divRoundUp(sizeZero, PAGE_SIZE);
  sizeZero = numPagesZero * PAGE_SIZE;

	size = sizeData + sizeCode + sizeZero;
	numPages = numPagesCode + numPagesData + numPagesZero;

  printf("Páginas totales %d, Data %d, Código %d, Otros %d\n",
  numPages, numPagesData, numPagesCode, numPagesZero);

	/* Controlo que el size sea menor o igual a 
   * la cantidad de bytes libres en bitmap*/
  //ASSERT(numPages <= bitmap->NumClear());
  // no debe explotar todo
    
  // Check we are not trying to run anything too big -- at least until we
  // have virtual memory.

  DEBUG('a', "Initializing address space, num pages %u, size %u\n",
        numPages, size);

  // First, set up the translation.

  pageTable = new TranslationEntry[numPages];

  for (unsigned i = 0; i < numPages; i++) {
    DEBUG('a', "Initializing address space, virtual page number %u\n",i);
    pageTable[i].virtualPage  = i;

    /* Al implementar carga por demanda pura, no debo
     * marcar las páginas como válidas al inicio, ni tampoco
     * reservar un marco de memoria.  */
    pageTable[i].valid        = false;
    pageTable[i].use          = false;
    #ifdef VMEM
    pageTable[i].swap         = false;
    #endif
    pageTable[i].dirty        = false;
    if (i < numPagesCode-1)
      pageTable[i].readOnly   = true;
    else
      pageTable[i].readOnly   = false;
    // If the code segment was entirely on a separate page, we could
    // set its pages to be read-only.
    
    /* Al implementar carga por demanda pura, no debo inicializar*/
    /* Inicializo en 0*/  
    //memset(machine->mainMemory + (pageTable[i].physicalPage)*PAGE_SIZE, 0, PAGE_SIZE);
    DEBUG('a', "Initializing to zero, physical page number %u\n",pageTable[i].physicalPage);    
  }
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
	for(unsigned i = 0; i < numPages; i++){
    if(pageTable[i].valid)
      bitmap->Clear(pageTable[i].physicalPage);
  }
  delete [] pageTable;
  delete exec;
}

/* Carga la página virtual virtualPage a memoria desde el ejecutable, si corresponde a
 * datos o texto. En caso contrario, inicializa en cero. */
void
AddressSpace::OnDemand(unsigned virtualPage)
{
  // Reservo un marco de memoria
  int frame = bitmap->Find();
  ASSERT(frame >= 0);
  pageTable[virtualPage].physicalPage = frame; 

  printf("Ocupando marco desde OnDemand %d - vpn %u\n",frame,virtualPage);
  /*Si la página a cargar pertenece al segmento de texto */
  if (virtualPage < numPagesCode)
    exec->ReadAt(&(machine->mainMemory[pageTable[virtualPage].physicalPage*PAGE_SIZE]),
                PAGE_SIZE, noffH.code.inFileAddr + virtualPage*PAGE_SIZE);
  else /* Si la página a cargar pertenece al segmento de datos */
    if (virtualPage < numPagesData + numPagesCode)
      exec->ReadAt(&(machine->mainMemory[pageTable[virtualPage].physicalPage*PAGE_SIZE]),
                  PAGE_SIZE, noffH.initData.inFileAddr + (virtualPage-numPagesCode)*PAGE_SIZE);
    else /* Inicializo en cero */
      memset(machine->mainMemory + (pageTable[virtualPage].physicalPage)*PAGE_SIZE, 0, PAGE_SIZE);
  
  pageTable[virtualPage].valid = true;    
}

#ifdef VMEM
/* Carga la página virtual virtualPage a memoria desde el swap */
void
AddressSpace::FromSwap(OpenFile *swap, unsigned virtualPage)
{
  // Reservo un marco de memoria
  int frame = bitmap->Find();
  ASSERT(frame >= 0);
  pageTable[virtualPage].physicalPage = frame; 
  //printf("Ocupando marco desde FromSwap %d - vpn %u\n",frame,virtualPage);

  int block = PAGE_SIZE*virtualPage;
  int read = swap->ReadAt(&(machine->mainMemory[pageTable[virtualPage].physicalPage*PAGE_SIZE]),
                           PAGE_SIZE,block);
  if(read != PAGE_SIZE)
    ASSERT(false);
  pageTable[virtualPage].use = false;
  pageTable[virtualPage].dirty = false;
  pageTable[virtualPage].valid = true;    
}
#endif
/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);

}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void AddressSpace::SaveState()
{
  #ifdef USE_TLB
    for(unsigned i = 0; i < TLB_SIZE; i++)
      if(machine->tlb[i].valid && machine->tlb[i].dirty){
        pageTable[machine->tlb[i].virtualPage].dirty = true;
        printf("Enciendo\n");
      }
  #endif  
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// Flush the TLB, setting valid bit in false.
void AddressSpace::RestoreState()
{
  #ifdef USE_TLB
  nextEntry = 0;
    for(unsigned i = 0; i < TLB_SIZE; i++)
      machine->tlb[i].valid = false;
  #endif
  
  //machine->pageTable     = pageTable;
  //machine->pageTableSize = numPages;
}

