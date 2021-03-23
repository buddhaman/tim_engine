
typedef struct 
{
    // Hyperparameters
    r32 dev;
    r32 learningRate;
    
    ui32 geneSize;
    ui32 nGenes;

    VecR32 *genes;
    VecR32 *epsilons;

    VecR32 *solution;

    VecR32 *fitness;
} EvolutionStrategies;
