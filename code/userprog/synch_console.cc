#include "synch_console.hh"

/// Console interrupt handlers.
///
/// Wake up the thread that requested the I/O.

static void ReadAvail(void *arg)
{
	SynchConsole *sc = (SynchConsole *)arg;
	sc->ReadConditionDone();
}

static void WriteDone(void *arg)
{
	SynchConsole *sc = (SynchConsole *) arg;
	sc->WriteConditionDone();
}

/*Defino la clase SynchCosole */

	SynchConsole::SynchConsole(char * debugName, const char *readFile, const char *writeFile){
		console = new Console (readFile, writeFile, ReadAvail, WriteDone, this),
		name = debugName;
		readAvail = new Semaphore ("Read avail", 0);
		writeDone = new Semaphore ("Read done", 0);
		readLock = new Lock ("Read lock");
		writeLock = new Lock ("Write lock");
	}
	
	SynchConsole::~SynchConsole(){
		//¿Hay que hacer delete de la consola?
		delete readAvail;
		delete writeDone;
		delete readLock;
		delete writeLock;
	}
	
	char
	SynchConsole::ReadChar(){
		readLock->Acquire();	
		console->CheckCharAvail();		
		/* Espero por la interrupción */
		readAvail->P();
		char ch = console->GetChar();
		readLock->Release();
		return ch;
	}
	
	void
	SynchConsole::WriteChar(char c){
		writeLock->Acquire();		
		console->PutChar(c);
		/*Espero por la interrupción*/
		writeDone->P();
		writeLock->Release();
	}
	
	/* Avisa que la condición de lectura se cumplió */
	void
	SynchConsole::ReadConditionDone(){
		readAvail->V();
	}
	
	/* Avisa que la condición de escritura terminada se cumplió */
	void
	SynchConsole::WriteConditionDone(){
		writeDone->V();
	}
