#define MAX_OPEN_FILE 50 /// Cantidad máxima de archivos abiertos por hilo

#include "syscall.h"
#include "../threads/list.hh"
class OpenFile;

class OpenFileTable {
public:

    /// Initialize the table
   OpenFileTable();

    /// De-allocate the data structures.
    ~OpenFileTable();

    /// Métodos

    /// Agrega un nuevo archivo a la tabla. A partir de un puntero a un objeto de OpenFile, obtengo un id.
    OpenFileId NewOpenFile(OpenFile *newOpenFile);

	/// Obtengo el OpenFile * de un archivo ya abierto anteriormente, a partir de su OpenFileId.
	OpenFile *GetOpenFile(OpenFileId fileID);
	
	/// Cierro un archivo
    void CloseFile(OpenFileId);

private:
	OpenFile** table;
	OpenFileId nextId;
	List<OpenFileId> *freeId;
};
