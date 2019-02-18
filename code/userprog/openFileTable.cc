#include "openFileTable.hh"

/* Implementación de la tabla de archivos abiertos */

    /// Initialize the table
   OpenFileTable::OpenFileTable(){
		nextId = 2;
		table = new OpenFile* [MAX_OPEN_FILE];
		freeId = new List<OpenFileId>;
   }

    /// De-allocate the data structures.
    OpenFileTable::~OpenFileTable(){
		delete table;
		delete freeId;
	}

    /// Métodos

    /// Agrega un nuevo archivo a la tabla. A partir de un puntero a un objeto de OpenFile, obtengo un id.
    OpenFileId
    OpenFileTable::NewOpenFile(OpenFile * newOpenFile){			
		/* Si tengo ids libres en la lista, uso el primero de ellos */
		if(!freeId->IsEmpty()){
			OpenFileId openFileId = freeId->Remove();
			table[openFileId] = newOpenFile;
			return openFileId;	
		}
		else{
			/* Si ya abrí la cantidad máxima de archivos, aviso y retorno -1 */
			if(nextId > MAX_OPEN_FILE){
				printf("No es posible abrir un nuevo archivo\n");
				return -1;
			}else{
			/* Utilizo el nextId */
				table[nextId] = newOpenFile;
				nextId ++;
				return nextId - 1;
			}
		}
	}

	/// Obtengo el OpenFile * de un archivo ya abierto anteriormente, a partir de su OpenFileId.
	OpenFile*
	OpenFileTable::GetOpenFile(OpenFileId fileID){
		/* Debo también recorrer la lista de ids libres por si es un id que ya se liberó */
		if(fileID >= nextId){
			printf("Id %d incorrecto \n", fileID);
			return NULL;
		}
		return table[fileID];			
	}
	
	/// Cierro un archivo
    void 
    OpenFileTable::CloseFile(OpenFileId fileID){
		if(fileID == nextId -1){
			nextId --;
			return;
		}
		freeId->Append(fileID);
		return;
	}
