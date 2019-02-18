#define MAX_PROCESS 50 /// Cantidad máxima de procesos

#include "syscall.h"
#include "../threads/list.hh"

class Thread;

class ProcessTable {
public:

    /// Initialize the table
   ProcessTable();

    /// De-allocate the data structures.
    ~ProcessTable();

    /// Métodos

    /// Agrega un nuevo proceso a la tabla.
    SpaceId NewProcess(Thread *newProcess);

	/// Obtengo el Thread * de un proceso, a partir de su SpaceId.
	Thread *GetProcess(SpaceId spaceID);
	
	/// Termino un proceso
    void DeleteProcess(SpaceId);

private:
	Thread** table;
	SpaceId nextId;
	List<SpaceId> *freeId;
};
