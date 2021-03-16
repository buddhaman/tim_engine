
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
    ui32 inputSize;
    ui32 hiddenSize;
    ui32 outputSize;
    ui32 stateSize;

    VecR32 *x; //input;
    VecR32 *f; //forget;
    VecR32 *h; //state;
    VecR32 *hc; //candidateState;
    VecR32 *bf; //forgetBias;
    VecR32 *bh; //stateBias;
    MatR32 *Wf; //inputForgetMatrix;
    MatR32 *Uf; //stateForgetMatrix;
    MatR32 *Wh; //inputCandidateStateMatrix;
    MatR32 *Uh; //forgottenStateCandidateStateMatrix;
} MinimalGatedUnit;


