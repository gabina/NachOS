/// Test program to do matrix multiplication on large arrays.
///
/// Intended to stress virtual memory system.
///
/// Ideally, we could read the matrices off of the file system, and store the
/// result back to the file system!


#include "syscall.h"


/// Sum total of the arrays does not fit in physical memory.
#define DIM  5

static int A[DIM][DIM];
static int B[DIM][DIM];
static int C[DIM][DIM];

int
main(void)
{
    int i, j, k;
    //int a = 3;
    //char HOLA[4];
    //HOLA[0] = 'a';
    //HOLA[1] = 'b';
	//HOLA[2] = 'c';
	//HOLA[3] = 'd';

	//Write("Inicializar\n", 12,1);
    // First initialize the matrices.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }

	//Write("Multiplicar\n", 12,1);	
    // Then multiply them together.
    for (i = 0; i < DIM; i++)
        for (j = 0; j < DIM; j++)
            for (k = 0; k < DIM; k++)
                C[i][j] += A[i][k] * B[k][j];

    // And then we are done.
    Exit(C[DIM - 1][DIM - 1]);
}
