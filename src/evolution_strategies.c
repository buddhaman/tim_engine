
size_t
SizeOfEvolutionStrategies(ui32 geneSize, ui32 nGenes)
{
    return sizeof(EvolutionStrategies) + SizeOfVecR32(geneSize)*(nGenes*2+1) + SizeOfVecR32(nGenes);
}

EvolutionStrategies *
ESCreate(MemoryArena *arena, ui32 geneSize, ui32 nGenes, r32 dev, r32 learningRate)
{
    size_t geneArraySize = sizeof(r32)*geneSize;
    size_t fitnessArraySize = sizeof(r32)*nGenes;
    EvolutionStrategies *strategies = PushStruct(arena, EvolutionStrategies);
    
    strategies->dev = dev;
    strategies->learningRate = learningRate;

    strategies->nGenes = nGenes;
    strategies->genes = PushArray(arena, VecR32, nGenes);
    strategies->epsilons = PushArray(arena, VecR32, nGenes);
    for(ui32 geneIdx = 0; 
            geneIdx < nGenes;
            geneIdx++)
    {
        InitVecR32(strategies->genes+geneIdx, geneSize, PushMemory_(arena, geneArraySize));
        InitVecR32(strategies->epsilons+geneIdx, geneSize, PushMemory_(arena, geneArraySize));
    }

    strategies->solution = PushStruct(arena, VecR32);
    InitVecR32(strategies->solution, geneSize, PushMemory_(arena, geneArraySize));

    strategies->fitness = PushStruct(arena, VecR32);
    InitVecR32(strategies->fitness, nGenes, PushMemory_(arena, fitnessArraySize));
    
    return strategies;
}

void
ESGenerateGenes(EvolutionStrategies *strategies)
{
    for(ui32 geneIdx = 0;
            geneIdx < strategies->nGenes;
            geneIdx++)
    {
        VecR32SetNormal(strategies->epsilons+geneIdx, strategies->dev);
        VecR32Add(strategies->genes+geneIdx, strategies->solution, strategies->epsilons+geneIdx);
    }
}

