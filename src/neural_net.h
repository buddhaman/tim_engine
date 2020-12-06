
typedef struct
{
    int inputSize;
    int outputSize;

    int nLayers;        // including input and output. So minimum of 2.
    VecR32 **layers;

    // For later
    VecR32 **biases;

    int nWeightMatrices;
    MatR32 **weights;
} FFNN;

