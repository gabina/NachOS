#include "processTable.hh"

/* Implementación de la tabla de procesos */

    /// Initialize the table
   ProcessTable::ProcessTable(){
		nextId = 0;
		table = new Thread* [MAX_PROCESS];
		freeId = new List<SpaceId>;
   }

    /// De-allocate the data structures.
    ProcessTable::~ProcessTable(){
		delete table;
		delete freeId;
	}

    /// Métodos

    /// Agrega un nuevo proceso la tabla. A partir de un puntero a un objeto de Thread, obtengo un id.
    SpaceId
    ProcessTable::NewProcess(Thread * newThread){	
		ASSERT(freeId!=NULL)
		/* Si tengo ids libres en la lista, uso el primero de ellos */
		if(!freeId->IsEmpty()){
			SpaceId spaceId = freeId->Remove();
			table[spaceId] = newThread;
			newThread->SetID(spaceId);
			return spaceId;
		}
		else{
			/* Si ya abrí la cantidad máxima de procesos, aviso y retorno -1 */
			if(nextId > MAX_PROCESS){
				printf("No es posible ejecutar un nuevo proceso\n");
				return -1;
			}else{
			/* Utilizo el nextId */
				table[nextId] = newThread;
				nextId ++;
				newThread->SetID(nextId - 1);
				return nextId - 1;
			}
		}
	}

	/// Obtengo el Thread * de un proceso ya guardado anteriormente, a partir de su SpaceId.
	Thread*
	ProcessTable::GetProcess(SpaceId spaceID){
		/* Debo también recorrer la lista de ids libres por si es un id que ya se liberó */
		if(spaceID >= nextId){
			printf("ProcessTable::GetProcess Id %d incorrecto \n", spaceID);
			return NULL;
		}
		return table[spaceID];			
	}
	
	/// Cierro un archivo
    void 
    ProcessTable::DeleteProcess(SpaceId spaceID){
		if(spaceID == nextId -1){
			nextId --;
			return;
		}
		freeId->Append(spaceID);
		return;
	}