
void
DoNeuralNetTests()
{
    MemoryArena *arena = CreateMemoryArena(128*1024*1024);

    ui32 inputSize = 1;
    ui32 outputSize = 1;
    ui32 hiddenSize = 1;
    ui32 geneSize = GetMinimalGatedUnitGeneSize(inputSize, outputSize, hiddenSize);
    ui32 nGenes = 12;
    r32 learningRate = 1;
    r32 dev = 0.2;
    size_t evolutionArenaSize = SizeOfEvolutionStrategies(geneSize, nGenes);

    // For testing evolution strategies.
#define QuickVec(size) CreateVecR32(size, PushAndZeroMemory_(arena, SizeOfVecR32(geneSize)))
    VecR32 *target = QuickVec(geneSize);
    for(int i = 0; i < geneSize; i++)
    {
        target->v[i] = 10+2*i;
    }

    DebugOut("Initializing evolution strategies with");
    DebugOut("input size = %d", inputSize);
    DebugOut("output size= %d", outputSize);
    DebugOut("hidden size= %d", hiddenSize);
    DebugOut("gene size= %d", geneSize);
    DebugOut("arena size = %zu", evolutionArenaSize);

    MemoryArena *evolutionSubArena = CreateSubArena(arena, evolutionArenaSize);

    EvolutionStrategies *strategies = ESCreate(evolutionSubArena, geneSize, nGenes, dev, learningRate);
    (void)strategies;

    for(int i = 0; i < strategies->geneSize; i++)
    {
        strategies->solution->v[i] = 0;
    }
    for(int generation = 0;
            generation < 200;
            generation++)
    {
        ESGenerateGenes(strategies);
        // calculate fitness
        for(int i = 0; i < strategies->nGenes; i++)
        {
            strategies->fitness->v[i] = -VecR32Distance2(strategies->genes+i, target);
        }

        ESNextSolution(strategies);

        VecR32PrintF(strategies->solution, 7, 3);
    }

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
    r32 sum = VecR32GetSum(vec);
    r32 avg = sum/n;
    r32 variance = 0;
    for(int i = 0; i < vec->n; i++)
    {
        r32 dev = vec->v[i]-avg;
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
