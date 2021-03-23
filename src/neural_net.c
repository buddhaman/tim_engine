
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

// In floats. Not in bytes.
ui32
GetMinimalGatedUnitGeneSize(int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize)
{
    ui32 stateSize = outputLayerSize+hiddenLayerSize;
    ui32 inputMatrixSize = inputLayerSize*stateSize;
    ui32 stateMatrixSize = stateSize*stateSize;
    ui32 biasSize = stateSize;
    return inputMatrixSize*2 + stateMatrixSize*2 + biasSize*2;
}

ui32
GetMinimalGatedUnitStateSize(int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize)
{
    ui32 stateSize = outputLayerSize+hiddenLayerSize;
    return stateSize *4 + inputLayerSize;
}

size_t
SizeOfMinimalGatedUnit(int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize)
{
    return sizeof(MinimalGatedUnit) + GetMinimalGatedUnitStateSize(inputLayerSize, 
            outputLayerSize, 
            hiddenLayerSize)*sizeof(r32);
}

internal inline VecR32 *
CreateMinimalGatedUnitGene(MemoryArena *arena, 
        int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize)
{
    ui32 size = GetMinimalGatedUnitGeneSize(inputLayerSize, outputLayerSize, hiddenLayerSize);
    void *memory = PushAndZeroMemory_(arena, SizeOfVecR32(size));
    VecR32 *vec = CreateVecR32(size, memory);
    for(int i = 0; i < size; i++)
    {
        vec->v[i] = RandomR32(-4, 4);
    }
    return vec;
}

MinimalGatedUnit*
CreateMinimalGatedUnit(MemoryArena *arena,
        int inputLayerSize,
        int outputLayerSize,
        int hiddenLayerSize,
        VecR32 *gene)
{
    MinimalGatedUnit *brain = PushStruct(arena, MinimalGatedUnit);

    brain->inputSize = inputLayerSize;
    brain->outputSize = outputLayerSize;
    brain->hiddenSize = hiddenLayerSize;
    brain->stateSize = brain->outputSize+brain->hiddenSize;
    brain->gene = gene;

    // Parameters from gene.
    r32 *atMemory = gene->v;
    InitVecR32FromGene(&brain->bf, brain->stateSize, gene, &atMemory);
    InitVecR32FromGene(&brain->bh, brain->stateSize, gene, &atMemory);
    InitMatR32FromGene(&brain->Wf, brain->inputSize, brain->stateSize, gene, &atMemory);
    InitMatR32FromGene(&brain->Wh, brain->inputSize, brain->stateSize, gene, &atMemory);
    InitMatR32FromGene(&brain->Uf, brain->stateSize, brain->stateSize, gene, &atMemory);
    InitMatR32FromGene(&brain->Uh, brain->stateSize, brain->stateSize, gene, &atMemory);

    r32 *atTransientMemory = PushArray(arena, r32, 
            GetMinimalGatedUnitStateSize(inputLayerSize, outputLayerSize, hiddenLayerSize));

    InitVecR32(&brain->x, brain->inputSize, atTransientMemory);
    atTransientMemory+=brain->inputSize;
    InitVecR32(&brain->f, brain->stateSize, atTransientMemory);
    atTransientMemory+=brain->stateSize;
    InitVecR32(&brain->h, brain->stateSize, atTransientMemory);
    atTransientMemory+=brain->stateSize;
    InitVecR32(&brain->hc, brain->stateSize, atTransientMemory);
    atTransientMemory+=brain->stateSize;

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

    VecR32 *x  = &brain->x; 
    VecR32 *f  = &brain->f;
    VecR32 *h  = &brain->h;
    VecR32 *hc = &brain->hc;
    VecR32 *bf = &brain->bf;
    VecR32 *bh = &brain->bh;
    MatR32 *Wf = &brain->Wf;
    MatR32 *Uf = &brain->Uf;
    MatR32 *Wh = &brain->Wh;
    MatR32 *Uh = &brain->Uh;

    // Forget
    MultiplyMatVecR32(tempv0, Wf, x);
    MultiplyMatVecR32(tempv1, Uf, h);
    VecR32Add(f, tempv0, tempv1);
    VecR32Add(f, f, bf);
    VecR32Apply(f, f, Sigmoid);

    // Candidate state
    MultiplyMatVecR32(tempv0, Wh, x);
    VecR32Hadamard(tempv2, f, h);
    MultiplyMatVecR32(tempv1, Uh, h);
    VecR32Add(hc, tempv0, tempv1);
    VecR32Add(hc, hc, bh);
    VecR32Apply(hc, hc, HyperbolicTangent);

    // New state
    VecR32Set(tempv0, f);
    VecR32MulS(tempv0, -1.0);
    VecR32AddS(tempv0, 1.0);
    VecR32Hadamard(tempv1, tempv0, h);
    VecR32Hadamard(tempv2, f, hc);
    VecR32Add(h, tempv1, tempv2);
}

void
MinimalGatedUnitPrintP(FILE *stream, MinimalGatedUnit *brain)
{
    fprintf(stream, "inputSize: %d\n", brain->inputSize);
    fprintf(stream, "outputSize: %d\n", brain->outputSize);
    fprintf(stream, "hiddenSize: %d\n", brain->hiddenSize);
    fprintf(stream, "parameters: %d\n", brain->gene->n);
}


