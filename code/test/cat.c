#include "syscall.h"

/*Simula un pequeño comando cat*/

static inline void
printFile(OpenFileId id){
    const OpenFileId OUTPUT = ConsoleOutput;
    char buffer[1];
    int read;
    read = Read(buffer, 1, id);
    while(read != 0){
        Write(buffer, 1, OUTPUT);
        read = Read(buffer, 1, id);
    }
    return;
}

int
main(int argc, char **argv)
{
    SpaceId newProc;
	const OpenFileId OUTPUT = ConsoleOutput;
	char str[10];
    str[0] ='E';
    str[1] = 'R';
    str[2] ='R';
    str[3] = 'O';
    str[4] ='R';       
    /*Si el único argumento es el nombre del comando, 
    se debería abrir una consola.
    Imprime error*/
    if (argc == 1)
        Write(str, 5, OUTPUT);
    else{
        OpenFileId files[argc-1];
        for(unsigned i = 1; i < argc; i++){
            files[i-1] = Open(argv[i]);
            printFile(files[i-1]);
        }
    }
	return 0;
}
