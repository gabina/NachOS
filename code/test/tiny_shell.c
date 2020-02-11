#include "syscall.h"


int
main(void)
{
    SpaceId    newProc, newProc2;
    OpenFileId input  = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char       prompt[2], ch, buffer[60], str[3];
    int        i;

    prompt[0] = '-';
    prompt[1] = '-';
	str[0] = 'O';
	str[1] = 'K';
	str[2] = '\n';
	
    while (1)
    {
        Write(prompt, 2, output);
        
        i = 0;
        do
		{
			//Write("Leo\n",4,output);
			Read(&buffer[i], 1, input);
			//Write(str,2,output);
			//Write(&buffer[i],1,output);
		}
        while (buffer[i++] != '\n');
        /*
		Write("Leo\n",4,output);
		Read(&buffer[0], 1, input);
		Write("Lei\n",4,output);
		Write(&buffer[0],1,output);
		
		while (buffer[i++] != '\n'){
			Write("Leo\n",4,output);
			Read(&buffer[i], 1, input);
			Write("Lei\n",4,output);
			Write(&buffer[i],1,output);
		}*/


		//Write("Sali\n",5,output);
        buffer[--i] = '\0';
	
		//Write(buffer,i,output);
	
        if (i > 0) {
            newProc = Exec(buffer);
            //newProc2 = Exec(buffer);
            //Write("Exec\n",5,output);
            Join(newProc);
        }
    }
}
