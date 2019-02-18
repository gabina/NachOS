/// Simple program to test whether running a user program works.
///
/// Prueba escribir en archivos. 
/// Crea uno, lo abre, lo escribe, lo cierra.
/// Vuelve a abrirlo, lee algo de él.
/// Escribe en el archivo 1 (consola)


#include "syscall.h"


int
main(void)
{
	char buffer[10];
    Create("test.txt");
    OpenFileId o = Open("test.txt");
    Write("La vida", 7,o);
    Close(o);
    o = Open("test.txt");    
    Read(buffer, 4, o);
    Close(o);
    Write(buffer, 4, 1);
}
