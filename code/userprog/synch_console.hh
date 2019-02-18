#include "threads/synch.hh"
#include "machine/console.hh"

/*Defino la clase SynchCosole */

class SynchConsole {
public:

	SynchConsole(char * debugName, const char *readFile, const char *writeFile);
	
	~SynchConsole();
	
	char
	ReadChar();
	
	void
	WriteChar(char c);
	
	void 
	ReadConditionDone();
	
	void
	WriteConditionDone();

private:
/// Data structures needed for the console test.
///
/// Threads making I/O requests wait on a `Semaphore` to delay until the I/O
/// completes.

	Console *console;
	char * name;
	Semaphore *writeDone;
	Semaphore *readAvail;
	Lock *readLock;
	Lock *writeLock;

};
