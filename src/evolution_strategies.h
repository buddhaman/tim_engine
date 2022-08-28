
typedef struct 
{
    // Hyperparameters
    R32 dev;
    R32 learningRate;
    
    U32 geneSize;
    U32 nGenes;

    VecR32 *genes;
    VecR32 *epsilons;

    VecR32 *solution;

    VecR32 *fitness;
} EvolutionStrategies;

