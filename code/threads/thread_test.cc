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

int shareVariable;
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
    
    for (unsigned num = 0; num < 5; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** Thread `%s` is running: iteration %d\n", name, num);
        shareVariable ++;
        //interrupt->SetLevel(oldLevel);
        //currentThread->Yield();
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

	shareVariable = 0;
    Thread *newThread = new Thread("Hijo", puerto);
	newThread->Fork(SimpleThread, (void *) "Hilo 1");
	newThread->Join();
	printf("La variable compartida toma el valor: %d \n",shareVariable);
}
