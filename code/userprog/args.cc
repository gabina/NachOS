#include "machine/machine.hh"
#include "threads/system.hh"

const unsigned MAX_ARG_COUNT  = 32;
const unsigned MAX_ARG_LENGTH = 128;

/* Lee de la dirección userAddress una cantidad byteCount y lo escribe en la dirección outString*/
void 
ReadBufferFromUser(int userAddress, char *outString, unsigned byteCount){

	int x;
	/* por qué -1?
	for(unsigned int i = 0; i < byteCount - 1; i++){*/
	for(unsigned int i = 0; i < byteCount; i++){
		machine->ReadMem(userAddress + i, 1, &x);
		outString[i] = (unsigned char) x;
	}

}

/* Lee de la dirección userAddress un string o una cantidad de maxByteCount y lo escribe en la dirección outString*/
void 
ReadStringFromUser(int userAddress, char *outString, unsigned maxByteCount){
	
	int x;
	unsigned int count = 0;
	machine->ReadMem(userAddress, 1,  &x);
	while (count < maxByteCount - 1 && (unsigned char) x != '\0'){
		machine->ReadMem(userAddress + count, 1, &x);
		outString[count] = (unsigned char) x;
		count ++;
		machine->ReadMem(userAddress + count, 1, &x);
	}	
	/* Agrego el fin de cadena si es necesario */
	if(count == maxByteCount -2)	
		outString[count] =  '\0';
}

/* Escribe byteCount del buffer en la direción de usuario userAddress */
void WriteBufferToUser(const char *buffer, int userAddress, unsigned byteCount){
	
	for(unsigned int i = 0; i < byteCount; i++)
		machine->WriteMem(userAddress + i, 1, *(buffer + i));	
}

/* Escribe el string string en la dirección de usuario userAddress*/
void WriteStringToUser(const char *string, int userAddress){
	
	unsigned int i = 0;
	while(string[i] != '\0'){
		machine->WriteMem(userAddress + i, 1, string[i]);
		i ++;
	}
	/* Escribo el fin de cadena */
	machine->WriteMem(userAddress + i, 1, string[i]);
}

void
WriteArgs(char **args)
{
    ASSERT(args != NULL);

    DEBUG('e', "Writing command line arguments into child process.\n");

    // Start writing the arguments where the current SP points.
    int args_address[MAX_ARG_COUNT];
    unsigned i;
    int sp = machine->ReadRegister(STACK_REG);
    for (i = 0; i < MAX_ARG_COUNT; i++) {
        if (args[i] == NULL)        // If the last was reached, terminate.
            break;
        sp -= strlen(args[i]) + 1;  // Decrease SP (leave one byte for \0).
        WriteStringToUser(args[i], sp);  // Write the string there.
        args_address[i] = sp;       // Save the argument's address.
        delete args[i];             // Free the memory.
    }
    ASSERT(i < MAX_ARG_COUNT);

    sp -= sp % 4;     // Align the stack to a multiple of four.
    sp -= i * 4 + 4;  // Make room for the array and the trailing NULL.
    for (unsigned j = 0; j < i; j++)
        // Save the address of the j-th argument counting from the end down
        // to the beginning.
        machine->WriteMem(sp + 4 * j, 4, args_address[j]);
    machine->WriteMem(sp + 4 * i, 4, 0);  // The last is NULL.
    sp -= 16;  // Make room for the “register saves”.

    machine->WriteRegister(STACK_REG, sp);
    delete args;  // Free the array.
}

char **
SaveArgs(int address)
{
    ASSERT(address != 0);

    // Count the number of arguments up to NULL.
    int val;
    unsigned i = 0;
    do {
        machine->ReadMem(address + i * 4, 4, &val);
        i++;
    } while (i < MAX_ARG_COUNT && val != 0);
    if (i == MAX_ARG_COUNT && val != 0)
        // The maximum number of arguments was reached but the last is not
        // NULL.  Return NULL as error.
        return NULL;

    DEBUG('e', "Saving %u command line arguments from parent process.\n", i);

    char **ret = new char * [i];  // Allocate an array of `i` pointers. We
                                  // know that `i` will always be at least 1.
    for (unsigned j = 0; j < i - 1; j++) {
        // For each pointer, read the corresponding string.
        ret[j] = new char [MAX_ARG_LENGTH];
        machine->ReadMem(address + j * 4, 4, &val);
        ReadStringFromUser(val, ret[j], MAX_ARG_LENGTH);
    }
    ret[i - 1] = NULL;  // Write the trailing NULL.

    return ret;
}
