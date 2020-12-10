#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "src/tim_types.h"
#include "src/cool_memory.h"
#include "src/cool_memory.c"
#include "src/linalg.h"
#include "src/linalg.c"
#include "src/neural_net.h"
#include "src/neural_net.c"

int
main()
{
    srand(time(0));
    int w = 12;
    int h = 12;

#if 0
    MatR32 *mat0 = CreateMatR32(w, h, malloc(SizeOfMatR32(w, h)));
    mat0->m[w] = 10;
    MatR32PrintF(mat0, 10, 3);

    int n = 12;
    VecR32 *vec0 = CreateVecR32(n, malloc(SizeOfVecR32(n)));
    VecR32TransposePrintF(vec0, 10, 3);

    MatR32ClearToZero(mat0);
    MatR32SetToIdentity(mat0);
    MatR32PrintF(mat0, 10, 3);

    VecR32 *vec1 = CreateVecR32(n, malloc(SizeOfVecR32(n)));
    vec0->v[2] = 2.0;
    MultiplyMatVecR32(mat0, vec0, vec1);

    VecR32TransposePrintF(vec1, 10, 3);

    printf("\n");
    VecR32MulS(vec1, 2.5);

    VecR32TransposePrintF(vec1, 10, 3);

    FFNN *brain = CreateFFNN();

    for(int i = 0; i < brain->inputSize; i++)
    {
       brain->layers[0]->v[i] = i+1;
    }
    UpdateFFNN(brain);

    VecR32PrintF(brain->layers[0], 8, 3);
    MatR32PrintF(brain->weights[0], 8, 3);
    VecR32PrintF(brain->layers[1], 8, 3);

    printf("Test done. I am proud. It works\n");
#endif
}

