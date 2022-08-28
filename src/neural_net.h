
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

typedef struct
{
    U32 inputSize;
    U32 hiddenSize;
    U32 outputSize;
    U32 stateSize;
    VecR32 *gene;

    VecR32 x; //input;
    VecR32 f; //forget;
    VecR32 h; //state;
    VecR32 hc; //candidateState;
    VecR32 bf; //forgetBias;
    VecR32 bh; //stateBias;
    MatR32 Wf; //inputForgetMatrix;
    MatR32 Uf; //stateForgetMatrix;
    MatR32 Wh; //inputCandidateStateMatrix;
    MatR32 Uh; //forgottenStateCandidateStateMatrix;
} MinimalGatedUnit;


