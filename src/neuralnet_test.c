
internal inline void
EvaluateBrain(MinimalGatedUnit *brain, U32 steps, VecR32 *result)
{
    for(U32 i = 0; i < steps; i++)
    {
        brain->x.v[0] = ((R32)i)/steps;
        UpdateMinimalGatedUnit(brain);
        R32 output = brain->h.v[brain->stateSize-1];
        result->v[i] = output;
    }
}

void
RunBrainFromGene(U32 inputSize, U32 outputSize, U32 hiddenSize, VecR32 *gene, U32 steps, VecR32 *target)
{
    U32 stateSize = GetMinimalGatedUnitStateSize(inputSize, outputSize, hiddenSize);
    R32 *state = calloc(stateSize, sizeof(R32));
    MinimalGatedUnit *brain = calloc(1, sizeof(MinimalGatedUnit));
    VecR32 *result = CreateVecR32(steps, malloc(SizeOfVecR32(steps)));

    InitMinimalGatedUnit(brain, 
        inputSize, 
        outputSize, 
        hiddenSize, 
        gene,
        state);

    EvaluateBrain(brain, steps, result);

    DebugOut("Result, target: ");
    VecR32PrintF(result, 6, 2);
    VecR32PrintF(target, 6, 2);

    free(state);
    free(brain);
    free(result);
}

void
DoNeuralNetTests()
{
    MemoryArena *arena = CreateMemoryArena(128*1024*1024);

    U32 inputSize = 1;
    U32 outputSize = 1;
    U32 hiddenSize = 1;
    U32 geneSize = GetMinimalGatedUnitGeneSize(inputSize, outputSize, hiddenSize);
    U32 nGenes = 52;
    U32 brainStateSize = GetMinimalGatedUnitStateSize(inputSize, outputSize, hiddenSize);
    R32 learningRate = 0.005;
    R32 dev = 0.05;
    U32 nGenerations = 1000;
    size_t evolutionArenaSize = SizeOfEvolutionStrategies(geneSize, nGenes);

    // For testing evolution strategies.
#define QuickVec(size) CreateVecR32(size, PushAndZeroMemory_(arena, SizeOfVecR32(geneSize)))
    U32 timeSteps = 10;
    VecR32 *target = QuickVec(timeSteps);
    for(int i = 0; i < timeSteps; i++)
    {
        target->v[i] = i%2;
    }

    DebugOut("Initializing evolution strategies with");
    DebugOut("input size = %d", inputSize);
    DebugOut("output size = %d", outputSize);
    DebugOut("hidden size = %d", hiddenSize);
    DebugOut("gene size = %d", geneSize);
    DebugOut("arena size = %zu", evolutionArenaSize);

    MemoryArena *evolutionSubArena = CreateSubArena(arena, evolutionArenaSize);

    EvolutionStrategies *strategies = ESCreate(evolutionSubArena, geneSize, nGenes, dev, learningRate);

    MinimalGatedUnit *brains = PushArray(arena, MinimalGatedUnit, nGenes);
    R32 *brainStates = PushArray(arena, R32, brainStateSize*nGenes);

    for(int i = 0; i < strategies->geneSize; i++)
    {
        strategies->solution->v[i] = 0;
    }
    VecR32 *result = QuickVec(timeSteps);
    for(int generation = 0;
            generation < nGenerations;
            generation++)
    {
        ESGenerateGenes(strategies);

        // Build brains
        memset(brainStates, 0, brainStateSize*nGenes);
        for(U32 brainIdx = 0;
                brainIdx < nGenes;
                brainIdx++)
        {
            MinimalGatedUnit *brain = brains+brainIdx;
            InitMinimalGatedUnit(brain, 
                    inputSize, 
                    outputSize, 
                    hiddenSize, 
                    strategies->genes+brainIdx,
                    brainStates+brainIdx*brainStateSize);
            // Run world
            EvaluateBrain(brain, timeSteps, result);
            R32 fitness = 0;
            for(int i = 0; i < timeSteps; i++)
            {
                R32 diff = result->v[i]-target->v[i];
                fitness-=diff*diff;
            }
            strategies->fitness->v[brainIdx] = fitness;
        }

        DebugOut("At generation %d fitness = %f", generation, VecR32Average(strategies->fitness));

        ESNextSolution(strategies);

        //VecR32PrintF(strategies->solution, 7, 3);
    }
    //VecR32PrintF(strategies->solution, 7, 3);
    RunBrainFromGene(inputSize, outputSize, hiddenSize, strategies->solution, timeSteps, target);

#if 0
    VecR32PrintF(strategies->genes, 7, 3);
    VecR32PrintF(strategies->genes+1, 7, 3);
    VecR32PrintF(strategies->genes+2, 7, 3);
#endif
    
#if 0
    DebugOut("Hello");
    Vec2 v = RandomNormalPair();

    DebugOut("%f %f", v.x, v.y);

    // See if the stddev and average are 1, 0 respectively.
    int n = 29;
    VecR32 *vec = CreateVecR32(n, PushAndZeroMemory_(arena, SizeOfVecR32(n)));
    VecR32SetNormal(vec);
    VecR32TransposePrintF(vec, 7, 3);
    R32 sum = VecR32GetSum(vec);
    R32 avg = sum/n;
    R32 variance = 0;
    for(int i = 0; i < vec->n; i++)
    {
        R32 dev = vec->v[i]-avg;
        variance+=(dev*dev);
    }
    variance/=n;
    DebugOut("avg: %f , variance: %f", avg, variance);
#endif

#if 0
    MinimalGatedUnit *brain = CreateMinimalGatedUnit(arena, 6, 2, 2);

    for(int i = 0; i < brain->inputSize; i++)
    {
        brain->x->v[i] = 1.0;
    }
    VecR32PrintF(brain->bf, 5, 2);
    VecR32PrintF(brain->bh, 5, 2);
    for(int i = 0; i < 30; i++)
    {
        for(int inputIdx = 0; inputIdx < brain->inputSize; inputIdx++)
        {
            brain->x->v[inputIdx] = sinf(i*0.1+inputIdx*0.2);
        }
        UpdateMinimalGatedUnit(brain);
        //VecR32PrintF(brain->x, 5, 2);
        VecR32PrintF(brain->h, 5, 2);
    }
#endif

#undef QuickVec

}
