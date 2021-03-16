
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
        MultiplyMatVecR32(to, weights, from);
        // Relu
        for(int i = 0; i < to->n; i++)
        {
            to->v[i] = to->v[i] < 0 ? 0 : to->v[i];
        }
    }
}

VecR32 * 
CreateBrainVector(MemoryArena *arena, int vecSize)
{
    return CreateVecR32(vecSize, PushAndZeroMemory_(arena, SizeOfVecR32(vecSize)));
}

MatR32 * 
CreateBrainMatrix(MemoryArena *arena, int w, int h)
{
    return CreateMatR32(w, h, PushAndZeroMemory_(arena, SizeOfMatR32(w, h)));
}

MinimalGatedUnit*
CreateMinimalGatedUnit(MemoryArena *arena, 
        int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize)
{
    MinimalGatedUnit *brain = PushStruct(arena, MinimalGatedUnit);

    brain->inputSize = inputLayerSize;
    brain->outputSize = outputLayerSize;
    brain->hiddenSize = hiddenLayerSize;
    brain->stateSize = brain->outputSize+brain->hiddenSize;

    brain->x = CreateBrainVector(arena, brain->inputSize);
    brain->f  = CreateBrainVector(arena, brain->stateSize);
    brain->h  = CreateBrainVector(arena, brain->stateSize);
    brain->hc = CreateBrainVector(arena, brain->stateSize);
    brain->bf = CreateBrainVector(arena, brain->stateSize);
    brain->bh = CreateBrainVector(arena, brain->stateSize);
    brain->Wf = CreateBrainMatrix(arena, brain->inputSize, brain->stateSize);
    brain->Uf = CreateBrainMatrix(arena, brain->stateSize, brain->stateSize);
    brain->Wh = CreateBrainMatrix(arena, brain->inputSize, brain->stateSize);
    brain->Uh = CreateBrainMatrix(arena, brain->stateSize, brain->stateSize);

    // Populate with random values
    r32 dev = 2.0;
    RandomVecR32(brain->bf, -dev, dev);
    RandomVecR32(brain->bh, -dev, dev);
    RandomMatR32(brain->Wf, -dev, dev);
    RandomMatR32(brain->Uf, -dev, dev);
    RandomMatR32(brain->Wh, -dev, dev);
    RandomMatR32(brain->Uh, -dev, dev);

    return brain;
}

void
UpdateMinimalGatedUnit(MinimalGatedUnit *brain)
{
    // Allocate temporary vectors on the stack.
    ui8 tempMem0[SizeOfVecR32(brain->stateSize)];
    VecR32 *tempv0 = CreateVecR32(brain->stateSize, (void*)tempMem0);
    ui8 tempMem1[SizeOfVecR32(brain->stateSize)];
    VecR32 *tempv1 = CreateVecR32(brain->stateSize, (void*)tempMem1);
    ui8 tempMem2[SizeOfVecR32(brain->stateSize)];
    VecR32 *tempv2 = CreateVecR32(brain->stateSize, (void*)tempMem2);

    // Forget
    MultiplyMatVecR32(tempv0, brain->Wf, brain->x);
    MultiplyMatVecR32(tempv1, brain->Uf, brain->h);
    VecR32Add(brain->f, tempv0, tempv1);
    VecR32Add(brain->f, brain->f, brain->bf);
    VecR32Apply(brain->f, brain->f, Sigmoid);

    // Candidate state
    MultiplyMatVecR32(tempv0, brain->Wh, brain->x);
    VecR32Hadamard(tempv2, brain->f, brain->h);
    MultiplyMatVecR32(tempv1, brain->Uh, brain->h);
    VecR32Add(brain->hc, tempv0, tempv1);
    VecR32Add(brain->hc, brain->hc, brain->bh);
    VecR32Apply(brain->hc, brain->hc, HyperbolicTangent);

    // New state
    VecR32Set(tempv0, brain->f);
    VecR32MulS(tempv0, -1.0);
    VecR32AddS(tempv0, 1.0);
    VecR32Hadamard(tempv1, tempv0, brain->h);
    VecR32Hadamard(tempv2, brain->f, brain->hc);
    VecR32Add(brain->h, tempv1, tempv2);
}

