
size_t
SizeOfEvolutionStrategies(U32 geneSize, U32 nGenes)
{
    return sizeof(EvolutionStrategies) + SizeOfVecR32(geneSize)*(nGenes*2+1) + SizeOfVecR32(nGenes);
}

EvolutionStrategies *
ESCreate(MemoryArena *arena, U32 geneSize, U32 nGenes, R32 dev, R32 learningRate)
{
    size_t geneArraySize = sizeof(R32)*geneSize;
    size_t fitnessArraySize = sizeof(R32)*nGenes;
    EvolutionStrategies *strategies = PushStruct(arena, EvolutionStrategies);
    
    strategies->geneSize = geneSize;
    strategies->nGenes = nGenes;
    strategies->dev = dev;
    strategies->learningRate = learningRate;

    strategies->genes = PushArray(arena, VecR32, nGenes);
    strategies->epsilons = PushArray(arena, VecR32, nGenes);
    for(U32 geneIdx = 0; 
            geneIdx < nGenes;
            geneIdx++)
    {
        InitVecR32(strategies->genes+geneIdx, geneSize, PushMemory_(arena, geneArraySize));
        InitVecR32(strategies->epsilons+geneIdx, geneSize, PushMemory_(arena, geneArraySize));
    }

    // Strategies solution. Initialize with something random.
    strategies->solution = PushStruct(arena, VecR32);
    InitVecR32(strategies->solution, geneSize, PushAndZeroMemory_(arena, geneArraySize));
    R32 invSquare = 10.0/sqrtf(geneSize);
    VecR32SetNormal(strategies->solution, invSquare);

    strategies->fitness = PushStruct(arena, VecR32);
    InitVecR32(strategies->fitness, nGenes, PushAndZeroMemory_(arena, fitnessArraySize));
    
    return strategies;
}

void
ESGenerateGenes(EvolutionStrategies *strategies)
{
    for(U32 geneIdx = 0;
            geneIdx < strategies->nGenes;
            geneIdx++)
    {
        VecR32SetNormal(strategies->epsilons+geneIdx, strategies->dev);
        VecR32Add(strategies->genes+geneIdx, strategies->solution, strategies->epsilons+geneIdx);
    }
}

void
ESNextSolution(EvolutionStrategies *strategies)
{
    size_t tempGeneSize = SizeOfVecR32(strategies->geneSize);
    U8 tempGeneMem0[tempGeneSize];
    VecR32 *tempGene = CreateVecR32(strategies->geneSize, tempGeneMem0);

    size_t tempFitnessSize = SizeOfVecR32(strategies->nGenes);
    U8 tempFitnessMem0[tempFitnessSize];
    VecR32 *tempFitness = CreateVecR32(strategies->nGenes, tempFitnessMem0);
    (void)tempFitness;

    VecR32SetS(tempGene, 0);

    R32 fitnessMean = VecR32Average(strategies->fitness);
    R32 fitnessDev = sqrtf(VecR32Variance(strategies->fitness));
    VecR32AddS(tempFitness, strategies->fitness, -fitnessMean);
    VecR32MulS(tempFitness, tempFitness, 1.0/fitnessDev);
    // Sum all epsilons scaled by fitness. Same as outer product.
    for(int geneIdx = 0; 
            geneIdx < strategies->nGenes;
            geneIdx++)
    {
        VecR32AddScaled(tempGene, tempGene, strategies->epsilons+geneIdx, tempFitness->v[geneIdx]);
    }
    R32 factor = strategies->learningRate/(strategies->nGenes*strategies->dev);
    VecR32AddScaled(strategies->solution, strategies->solution, tempGene, factor);
}


