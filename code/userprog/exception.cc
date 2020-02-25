/// Entry point into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "syscall.h"
#include "threads/system.hh"
#include "args.cc"

#define MAX_NAME 64
#define MAX_ARGS 8

int nProc;
void processCreator(void * arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState(); // load page table register
	//Escribo los argumentos en la pila
	if (arg!= NULL){
		int *ret = WriteArgs((char **) arg);
		machine->WriteRegister(4, ret[0]);
		machine->WriteRegister(5, ret[1]);
	}
	machine->Run(); // jump to the user progam
	ASSERT(false); // machine->Run never returns;
}

/// Entry point into the Nachos kernel.  Called when a user program is
/// executing, and either does a syscall, or generates an addressing or
/// arithmetic exception.
///
/// For system calls, the following is the calling convention:
///
/// * system call code in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the pc before returning. (Or else you will
/// loop making the same system call forever!)
///
/// * `which` is the kind of exception.  The list of possible exceptions is
///   in `machine.hh`.
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

	switch (which){
		case SYSCALL_EXCEPTION:{
			switch(type){
				case SC_Halt:{
					DEBUG('a', "Shutdown, initiated by user program.\n");
					interrupt->Halt();
					}
					break;
				case SC_Create:{
					int direc = machine->ReadRegister(4);
					char* nombre = new char [MAX_NAME];
					ReadStringFromUser(direc, nombre, MAX_NAME);
					bool ok = fileSystem->Create(nombre, 0);
					if (!ok)
						DEBUG('a',"File %s has not been created",nombre);
					}
					break;
				case SC_Open:{
					int direc = machine->ReadRegister(4);
					char* nombre = new char [MAX_NAME];
					ReadStringFromUser(direc, nombre, MAX_NAME);
					OpenFile * openFile = fileSystem->Open(nombre);
					OpenFileTable* fileTable = currentThread->GetTable();
					OpenFileId idFile = fileTable->NewOpenFile(openFile);
					DEBUG('a',"File %s has been opened\n",nombre);
					machine->WriteRegister(2, idFile);
					}
					break;
				case SC_Read:{
					int buffer = machine->ReadRegister(4);
					int size = machine->ReadRegister(5);
					OpenFileId idFile =(OpenFileId) machine->ReadRegister(6);
					int sizeRead;
					char * localBuffer = new char [size];
					
					/* Si el archivo a leer es la consola, utilizo la clase SynchConsole*/
					if(idFile == 0){
						char ch;
						for (int i = 0; i < size; i++){
							ch = synchChonsole->ReadChar();
							localBuffer[i] = ch;
						}
						sizeRead = size;
					}
					/* En otro caso, utilizo la clase openFile */
					else{
						/* Si es 1, devuelvo error*/
						if(idFile == 1){
							DEBUG('a',"Error\n");
							currentThread->Finish();
						}
						
						OpenFileTable* fileTable = currentThread->GetTable();
						if(fileTable == NULL){
							DEBUG('a'," null pointer\n");
							currentThread->Finish();
						}
						/* A partir del OpenFileId idFile obtengo un puntero a un objeto de OpenFile*/
						OpenFile* openFile =  fileTable->GetOpenFile(idFile);
						if(openFile == NULL){
							DEBUG('a',"Error: null pointer\n");
							currentThread->Finish();
						}
						sizeRead = openFile->Read(localBuffer, size);
					}
					/* Escribo en el buffer del usuario*/
					WriteBufferToUser(localBuffer, buffer, sizeRead);
					/*Retorno la cantidad de bytes leídos
					0 indica EOF */
					machine->WriteRegister(2, sizeRead);

					}
					break;
				case SC_Write:{
					int buffer = machine->ReadRegister(4);
					int size = machine->ReadRegister(5);
					OpenFileId idFile =(OpenFileId) machine->ReadRegister(6);
					int sizeWritten, i;
					char * localBuffer = new char [size];	
					ReadBufferFromUser(buffer, localBuffer, size);
					if(idFile == 1){
						for (i = 0; i < size; i++)
							synchChonsole ->WriteChar(localBuffer[i]);
						sizeWritten = size;
					}
					else{
						/* Si es 0, devuelvo error*/
						if(idFile == 0){
							DEBUG('a',"Error\n");
							printf("No entró bien\n");
							currentThread->Finish();

						}
						OpenFileTable* fileTable = currentThread->GetTable();
						/* A partir del OpenFile idFile obtengo un puntero a un objeto de OpenFile*/
						OpenFile* openFile =  fileTable->GetOpenFile(idFile);
						if(openFile == NULL){
							DEBUG('a',"Error: null pointer2\n");
							currentThread->Finish();
						}
						sizeWritten = openFile->Write(localBuffer, size);
					}
					machine->WriteRegister(2, sizeWritten);						
					}
					break;
				case SC_Close:{
					OpenFileId fileID = (OpenFileId) machine->ReadRegister(4);
					OpenFileTable* fileTable = currentThread->GetTable();
					fileTable->CloseFile(fileID);
					}
					break;
				case SC_Exit:{
					int status = machine->ReadRegister(4);
					printf("Status %d\n",status);
					/*Avisarle de algún modo al padre el status*/
					currentThread->Finish(status);				
					}
					break;
				case SC_Join:{
					SpaceId spaceID = (SpaceId) machine->ReadRegister(4);
					Thread* t = processTable->GetProcess(spaceID);
					int s = t->Join();
					machine->WriteRegister(2, s);	
					}
					break;
				case SC_Exec:{
					/* Leo el argumento. El nombre del archivo y el parámetro con el cual llamarlo, si hubiere */
					int name = machine->ReadRegister(4);
					int args = machine->ReadRegister(5);
					char *nombre = new char [MAX_NAME];
					char **localArgs = new char * [MAX_ARGS];
					ReadStringFromUser(name, nombre, MAX_NAME);

					if (args)
						localArgs = SaveArgs(args);
					else
						localArgs = NULL;

					/* Abro el archivo */			
					OpenFile *executable = fileSystem->Open(nombre);
					if(executable==NULL){
						printf("Error al intentar abrir archivo %s\n",nombre);
						printf("Proceso %s terminado\n",currentThread->GetName());
						currentThread->Finish();
						break;
						//terminar el proceso
						//o avisar fallo
					}
					delete nombre;

					/* Reservo espacio*/
					AddressSpace *space;
					space = new AddressSpace(executable);

					/* Creo el nuevo hilo y asigno el espacio */
					Port * dadPort = currentThread->GetPort();
					char* procName = new char [20];
					snprintf(procName, 20, "user prog_%d", (int) (nProc++));
					Thread * t = new Thread(procName, dadPort);
					t->space = space;
					/* spaceID no está seteado al spaceID del thread*/
					SpaceId spaceID = processTable->NewProcess(t);

					#ifdef VMEM
					// Creo el archivo SWAP
					char* fileName = new char [20];
					snprintf(fileName, 20, "SWAP._%d", (int) (t->spaceId));
					if(!fileSystem->Create(fileName, 0)){
						printf("Error al crear archivo del SWAP\n");
						ASSERT(false);
					}
					#endif

					/* Retorno el spaceID*/
					machine->WriteRegister(2, spaceID);
					t->Fork(processCreator,localArgs);
					}
					break;
				default:
					printf("Unexpected user mode exception %d %d\n", which, type);
					ASSERT(false);
					break;				
			}
			/* Acomodo los registros */
			int pc; 
			pc = machine->ReadRegister(PC_REG); 
			machine->WriteRegister(PREV_PC_REG,pc); 
			pc = machine->ReadRegister(NEXT_PC_REG); 
			machine->WriteRegister(PC_REG,pc); 
			pc += 4; 
			machine->WriteRegister(NEXT_PC_REG,pc); 
			break;
		}
		case PAGE_FAULT_EXCEPTION:{
			/*La página virtual buscada no está en el TLB. 
			Debe buscar en la memoria cuál es el marco correspondiente. 
			La nueva entrada debe ser agregada a la TLB*/

			#ifdef USE_TLB
				// Cuento un nuevo miss
				if(ratio)					
					misses ++;
				// Leo la dirección que produjo la falla
				int virtAddr = machine->ReadRegister(BAD_VADDR_REG);
				// Recupero tabla de páginas
				TranslationEntry *pageTable = (currentThread->space)->pageTable;

				// Calculate the virtual page number from the virtual address.
				unsigned vpn = (unsigned) virtAddr / PAGE_SIZE;
				printf("PAGE FAULT\n");
				/* Si la página no está marcada en la tabla como válida,
				* entonces no está cargada en memoria y debo hacerlo*/
				if (!pageTable[vpn].valid){
					printf("La página no está cargada en memoria\n");
					printf("%u\n",bitmap->NumClear());
					#ifdef VMEM
					printf("hola\n");
					/* Si no hay marcos de memoria física disponibles*/
					if(bitmap->NumClear() == 0){
						printf("Hola\n");
						Victim *victim = GiveVictim(victims);
						Thread *thread = GetProcess(victim->process);
						TranslationEntry* pageTable = (thread->space)->pageTable;
						unsigned physicalPage = pageTable[victim->virtualPage];
						//TO DO: copiar la página en el archivo correspondiente y liberarla

						//Abrir el archivo si aún no está abierto
						if(thread->swap==NULL){
							char* fileName = new char [20];
							snprintf(fileName, 20, "SWAP._%d", (int) (thread->spaceId));
							thread->swap = fileSystem->Open(fileName);
							delete fileName;
						}
						int block = PAGE_SIZE*(victim->virtualPage);
						(thread->swap)->WriteAt(&(machine->mainMemory[physicalPage]),
												PAGE_SIZE,block);
						bitMap->Clear(physicalPage);
					}
					Victim *newVictim = new Victim;
					newVictim->process = currentThread->GetSpaceId();
					newVictim->virtualPage = vpn; 
					victims->Append(newVictim);
					#endif
					(currentThread->space)->OnDemand(vpn);
				}
				// Cargo la entrada al TLB
				(machine->tlb)[nextEntry] = pageTable[vpn];
				nextEntry = (nextEntry+1) %	TLB_SIZE;
			#endif
			}
			break;
		case READ_ONLY_EXCEPTION:{
			currentThread->Finish();
			break;
		}
		case ILLEGAL_INSTR_EXCEPTION:{
			printf("Instrucción ilegal\n");
			currentThread->Finish();
			break;
		}
	}
}
