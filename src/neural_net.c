
FFNN*
CreateFFNN()
{
    FFNN *brain = malloc(sizeof(FFNN));

    brain->inputSize = 2;
    brain->outputSize = 4;
    brain->nLayers = 2;
    brain->layers = malloc(sizeof(VecR32*)*brain->nLayers);
    brain->layers[0]= CreateVecR32(brain->inputSize, malloc(SizeOfVecR32(brain->inputSize)));
    brain->layers[1]= CreateVecR32(brain->outputSize, malloc(SizeOfVecR32(brain->inputSize)));

    brain->nWeightMatrices = 1;
    brain->weights = malloc(sizeof(MatR32*)*brain->nWeightMatrices);
    brain->weights[0] = CreateMatR32(brain->inputSize, 
            brain->outputSize, 
            malloc(SizeOfMatR32(brain->inputSize, brain->outputSize)));
    RandomMatR32(brain->weights[0], -3, 3);

    return brain;
}

void
UpdateFFNN(FFNN *brain)
{
    for(int layerIdx = 0;
            layerIdx < brain->nLayers-1;
            layerIdx++)
    {
        VecR32 *from = brain->layers[layerIdx];
        VecR32 *to = brain->layers[layerIdx+1];
        MatR32 *weights = brain->weights[layerIdx];
        MultiplyMatVecR32(weights, from, to);
        // Relu
        for(int i = 0; i < to->n; i++)
        {
            to->v[i] = to->v[i] < 0 ? 0 : to->v[i];
        }
    }
}

