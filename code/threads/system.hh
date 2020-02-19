/// All global variables used in Nachos are defined here.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SYSTEM__HH
#define NACHOS_THREADS_SYSTEM__HH

#include<time.h> 
#include "utility.hh"
#include "thread.hh"
#include "scheduler.hh"
#include "userprog/bitmap.hh"
#include "machine/interrupt.hh"
#include "machine/statistics.hh"
#include "machine/timer.hh"


/// Initialization and cleanup routines.

// Initialization, called before anything else.
extern void Initialize(int argc, char **argv);

// Cleanup, called when Nachos is done.
extern void Cleanup();


extern Thread *currentThread;        ///< The thread holding the CPU.
extern Thread *threadToBeDestroyed;  ///< The thread that just finished.
extern Scheduler *scheduler;         ///< The ready list.
extern Interrupt *interrupt;         ///< Interrupt status.
extern Statistics *stats;            ///< Performance metrics.
extern Timer *timer;                 ///< The hardware alarm clock.
extern Port * mainPort;

#ifdef USER_PROGRAM
#include "machine/machine.hh"
#include "userprog/processTable.hh"
#include "userprog/synch_console.hh"
extern Machine* machine;  // User program memory and registers.
extern ProcessTable* processTable;
extern BitMap* bitmap;
extern SynchConsole * synchChonsole;
#endif

#ifdef FILESYS_NEEDED  // *FILESYS* or 8FILESYS_STUB*.
#include "filesys/file_system.hh"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "filesys/synch_disk.hh"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "network/post.hh"
extern PostOffice *postOffice;
#endif

#ifdef USE_TLB
extern bool ratio;
extern int misses;
extern int accesses;
#endif


#endif
