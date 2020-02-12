#include "syscall.h"

/*Simula un pequeño comando copy*/

static inline void
copyFile(OpenFileId src, OpenFileId dst){
    char buffer[1];
    int read;
    read = Read(buffer, 1, src);
    while(read != 0){
        Write(buffer, 1, dst);
        read = Read(buffer, 1, src);
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
    /*Si la cantidad de argumentos es 1 o 2, entonces no ingresé src y dst*/
    if (argc <3)
        Write(str, 5, OUTPUT);
    else{
        OpenFileId files[argc-2];
        OpenFileId dst = Open(argv[argc-1]);
        for(unsigned i = 1; i < argc-1; i++){
            files[i-1] = Open(argv[i]);
            copyFile(files[i-1], dst);
        }
    }
	return 0;
}
