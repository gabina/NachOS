/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"

const int CANT_THREADS = 1;
Port * puerto = new Port("Puerto 1");


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;
    
    int * buf = new int;   
    puerto->Receive(buf);
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 5; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %d\n", name, num);
        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("!!! Thread `%s` has finished\n", name);
    //interrupt->SetLevel(oldLevel);
}

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread2(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 5; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %d\n", name, num);
        if(num == 2)
         	puerto->Send(123);
        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("!!! Thread `%s` has finished\n", name);
    //interrupt->SetLevel(oldLevel);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest \n");

    Thread **newThread = new Thread * [CANT_THREADS];
    char **name = new char * [CANT_THREADS];
    for(int i = 0; i < CANT_THREADS; i++){
		name[i] = new char[64];
		sprintf(name[i],"Hilo %d ",i+1);	
		//newThread[i] = new Thread(name[i],NULL,9);
		newThread[i] = new Thread(name[i]);
		if(i % 2)
			newThread[i]->Fork(SimpleThread, (void *) name[i]);
		else
			newThread[i]->Fork(SimpleThread2, (void *) name[i]);
	}

    SimpleThread((void *) "main");
}
