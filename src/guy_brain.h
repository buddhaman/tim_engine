typedef struct
{
    Vec2 pos;
    Vec2 localPos;
    r32 radius;
    r32 tOffset;
    r32 xTRadius;
    r32 yTRadius;
} Neuron;

typedef struct
{
    int nNeurons;
    Neuron *neurons;

    r32 width;
} NeuronLayer;

typedef struct
{
    FFNN *brain;
    int nLayers;
    NeuronLayer *layers;
} GuyBrain;

