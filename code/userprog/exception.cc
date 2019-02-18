/// Entry point into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "syscall.h"
#include "threads/system.hh"
//#include "userprog/synch_console.hh"
#include "args.cc"

#define MAX_NAME 64

void processCreator(void * arg)
{
	currentThread->space->InitRegisters();
	currentThread->space->RestoreState(); // load page table register
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
	
/// Read `size` bytes from the open file into `buffer`.
///
/// Return the number of bytes actually read -- if the open file is not long
/// enough, or if it is an I/O device, and there are not enough characters to
/// read, return whatever is available (for I/O devices, you should always
/// wait until you can return at least one character).

// int Read(char *buffer, int size, OpenFileId id);

    int type = machine->ReadRegister(2);
    if (which == SYSCALL_EXCEPTION && type == SC_Halt) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    } else {
		switch(type){
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
					//SynchConsole *sc = new SynchConsole((char *) "Console 1", NULL, NULL);
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
						return;
					}
					
					OpenFileTable* fileTable = currentThread->GetTable();
					if(fileTable == NULL){
						DEBUG('a',"Error: null pointer\n");
						return;
					}
					/* A partir del OpenFileId idFile obtengo un puntero a un objeto de OpenFile*/
					OpenFile* openFile =  fileTable->GetOpenFile(idFile);
					if(openFile == NULL){
						DEBUG('a',"Error: null pointer\n");
						return;
					}
					sizeRead = openFile->Read(localBuffer, size);
				}
				/* Escribo en el buffer del usuario*/
				WriteBufferToUser(localBuffer, buffer, sizeRead);
				}
				break;
			case SC_Write:{
				int buffer = machine->ReadRegister(4);
				int size = machine->ReadRegister(5);
				OpenFileId idFile =(OpenFileId) machine->ReadRegister(6);
				int sizeWritten;
				char * localBuffer = new char [size];	
				ReadBufferFromUser(buffer, localBuffer, size);
				if(idFile == 1){
					//SynchConsole *sc = new SynchConsole((char *) "Console 1", NULL, NULL);
					for (int i = 0; i < size; i++)
						synchChonsole ->WriteChar(localBuffer[i]);
				}else{
					/* Si es 0, devuelvo error*/
					if(idFile == 0){
						DEBUG('a',"Error\n");
						return;
					}
					OpenFileTable* fileTable = currentThread->GetTable();
					/* A partir del OpenFile idFile obtengo un puntero a un objeto de OpenFile*/
					OpenFile* openFile =  fileTable->GetOpenFile(idFile);
					if(openFile == NULL){
						DEBUG('a',"Error: null pointer\n");
						return;
					}
					printf("Voy a escribir %s\n",localBuffer);
					sizeWritten = openFile->Write(localBuffer, size);
				}							
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
				/* Leo el argumento */
				printf("Estoy en exec\n");
				int direc = machine->ReadRegister(4);
				char* nombre = new char [MAX_NAME];
				ReadStringFromUser(direc, nombre, MAX_NAME);
				printf("Llamada a exec con %s \n",	nombre);
				/* Abro el archivo */			
				OpenFile *executable = fileSystem->Open(nombre);
				if(executable==NULL){
					printf("Dar un archivo válido\n");
					//terminar el proceso
					//o avisar fallo
				}
				/* Reservo espacio*/
				AddressSpace *space;
				space = new AddressSpace(executable);
				/* Creo el nuevo hilo y asigno el espacio */
				Thread * t = new Thread("user prog");
				t->space = space;
				/* spaceID no está seteado al spaceID del thread*/
				SpaceId spaceID = processTable->NewProcess(t);
				
				t->Fork(processCreator,NULL);
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
		
    }

    
}
