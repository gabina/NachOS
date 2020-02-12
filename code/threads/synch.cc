/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    Thread   *thread;
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    thread = queue->Remove();
    if (thread != NULL)  // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{
	sem = new Semaphore (debugName,1);
	name = debugName;
	threadChanged = NULL;
	threadLocker = NULL;
}

Lock::~Lock()
{
	delete sem;
}

void
Lock::Acquire()
{
	ASSERT(!IsHeldByCurrentThread());
	int myPrio = currentThread->GetPriority();

	/* Si la prioridad del que tiene el lock es menor a la mía y no se hizo inversión de prioridades, debo invertir prioridades*/
	if(threadLocker != NULL){
		int lockerPrio = threadLocker->GetPriority();
		if(lockerPrio < myPrio && lockerPrio == threadLocker->GetRealPriority()){
			currentThread->ChangePriority(myPrio, lockerPrio, threadLocker);
			threadChanged = currentThread;
		}
	}
	sem->P();
	threadLocker = currentThread;	
}

void
Lock::Release()
{
	ASSERT(IsHeldByCurrentThread());
	if(threadChanged != NULL)
		currentThread->ChangePriority(currentThread->GetPriority(), threadChanged->GetPriority(), threadChanged);
	threadLocker = NULL;	
	sem->V();
}

bool
Lock::IsHeldByCurrentThread()
{
	return (currentThread == threadLocker);
}

Condition::Condition(const char *debugName, Lock *conditionLock)
{
	name = debugName;
	/* Seteo el lock asociado*/
	condlock = conditionLock;
	/* Creo la lista de semáforos */
	queue = new List<Semaphore *>;
}
Condition::~Condition()
{
	Semaphore * s;
	delete condlock;
	/* Si algún hilo quedó dormido esperando */
	while(!queue->IsEmpty()){
		s = queue->Remove();
		delete s;
	}
}


void
Condition::Wait()
{
    /* Esto no se puede hacer
    condlock->Release();	
    queue->Append(currentThread); 
	currentThread->Sleep();  
	*/
	ASSERT(condlock->IsHeldByCurrentThread());	
	char * semName = new char [60];
	sprintf(semName,"SemaphoreCondition%s",GetName());
	/* Creo un nuevo semáforo en 0 */
	Semaphore* s = new Semaphore (semName,0);
	/* Lo agrego a la lista */
	queue->Append(s);
	/* Libero el lock*/
	condlock->Release();
	/* Aviso que me quiero dormir o me duermo */
	s->P();	
	/* Cuando me despierto, pido el lock */
	condlock->Acquire();
	delete s;
	
}

void
Condition::Signal()
{
	ASSERT(condlock->IsHeldByCurrentThread());
	Semaphore* s;
	if(!queue->IsEmpty()){
		s = queue->Remove();
		/* Despierto, o si todavía no se durmió, le aviso que se debe despertar*/
		s->V();
	}		
}

void
Condition::Broadcast()
{
	ASSERT(condlock->IsHeldByCurrentThread());	
	Semaphore* s;
	while(!queue->IsEmpty()){
		s = queue->Remove();
		/* Despierto, o si todavía no se durmió, le aviso que se debe despertar*/
		s->V();	
	}
}

Port::Port(const char * debugName)
{
	name = debugName;
	Lock* lock1 = new Lock("LockCondPort1");
	Lock* lock2 = new Lock("LockCondPort2");	
	firstCond = new Condition ("ConditionPort", lock1);
	secondCond = new Condition ("Condition2Port", lock2);
	buffer = new List<int *>;
}

Port::~Port()
{
	delete firstCond;
	delete secondCond;
	delete buffer;
}

void
Port::Send(int message)
{
	firstCond->GetLock()->Acquire();
	secondCond->GetLock()->Acquire();

	/* Si la lista de bufferes está vacía, entonces espero a que algún receptor me despierte */
	while(buffer->IsEmpty()){
		secondCond->GetLock()->Release();
		firstCond->Wait();
		secondCond->GetLock()->Acquire();
	}
	
	/* Escribo el mensaje en uno de los buffers*/
	if(!buffer->IsEmpty())
		*(buffer->Remove()) = message;
	/* Despierto al receptor */
	secondCond->Signal();		
	/* Ya escribí el mensaje. Libero el lock y retorno */
	firstCond->GetLock()->Release();
	secondCond->GetLock()->Release();		

}

void
Port::Receive(int * message)
{
	/* Tomo ambos locks */
	firstCond->GetLock()->Acquire();
	secondCond->GetLock()->Acquire();	
	/* Guardo en buffer la dirección en donde quiero recibir el mensaje */
	buffer->Append(message);
	/* Aviso que quiero recibir */
	firstCond->Signal();
	firstCond->GetLock()->Release();
	secondCond->Wait();
	/* Una vez despierto ya recibí el mensaje y retorno */
	secondCond->GetLock()->Release();		
	
}
