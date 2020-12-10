
FFNN*
CreateFFNN(MemoryArena *arena, 
        int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize,
        int nHiddenLayers)
{
    FFNN *brain = PushStruct(arena, FFNN);

    brain->inputSize = inputLayerSize;
    brain->outputSize = outputLayerSize;
    brain->nLayers = nHiddenLayers+2;
    brain->layers = PushArray(arena, VecR32*, brain->nLayers);
    brain->layers[0]= CreateVecR32(brain->inputSize, PushMemory_(arena, SizeOfVecR32(brain->inputSize)));
    brain->layers[brain->nLayers-1]= CreateVecR32(brain->outputSize, PushMemory_(arena, SizeOfVecR32(brain->inputSize)));
    for(int layerIdx = 1;
            layerIdx < brain->nLayers-1;
            layerIdx++)
    {
        brain->layers[layerIdx]= CreateVecR32(hiddenLayerSize, PushMemory_(arena, SizeOfVecR32(brain->inputSize)));
    }

    brain->nWeightMatrices = brain->nLayers-1;
    brain->weights = PushArray(arena, MatR32*, brain->nWeightMatrices);
    for(int weightIdx = 0; 
            weightIdx < brain->nWeightMatrices; 
            weightIdx++)
    {
        int w = brain->layers[weightIdx]->n;
        int h = brain->layers[weightIdx+1]->n;
        brain->weights[weightIdx] = CreateMatR32(w, h, PushMemory_(arena, SizeOfMatR32(w, h)));
    }
    RandomMatR32(brain->weights[0], -3, 3);

    return brain;
}

int
GetTotalNeuronCount(FFNN *brain)
{
    int sum = 0;
    for(int layerIdx = 0;
            layerIdx < brain->nLayers;
            layerIdx++)
    {
        sum+=brain->layers[layerIdx]->n;
    }
    return sum;
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

