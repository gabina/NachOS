/// Test routines for demonstrating that Nachos can load a user program and
/// execute it.
///
/// Also, routines for testing the Console hardware device.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

//#include "synch_console.hh"
#include "address_space.hh"
#include "threads/synch.hh"
#include "threads/system.hh"


/// Run a user program.
///
/// Open the executable, load it into memory, and jump to it.
void
StartProcess(const char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddressSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    space = new AddressSpace(executable);
    currentThread->space = space;

    delete executable;

    space->InitRegisters();  // Set the initial register values.
    space->RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user progam.
    ASSERT(false);   // `machine->Run` never returns; the address space
                     // exits by doing the system call `Exit`.
}



/// Test the console by echoing characters typed at the input onto the
/// output.
///
/// Stop when the user types a `q`.

/// Para probarla hacer: ./nachos -c

/// y comentar la SynchConsole del sistema
void
ConsoleTest(const char *in, const char *out)
{
	SynchConsole *sc = new SynchConsole((char *) "Console 1", in, out);
	char ch;
	for (;;){
		ch = sc->ReadChar();
		sc ->WriteChar(ch);
		if(ch == 'q')
			return;
	}
	
	
	
	/*
    char ch;
    console   = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
        readAvail->P();        // Wait for character to arrive.
        ch = console->GetChar();
        console->PutChar(ch);  // Echo it!
        writeDone->P();        // Wait for write to finish.
        if (ch == 'q')
            return;  // If `q`, quit.
    }*/
}

