void
InitGuyBrain(MemoryArena *arena, GuyBrain *brain)
{
    FFNN *ffnn = brain->brain;
    brain->layers = PushArray(arena, NeuronLayer, ffnn->nLayers);
    brain->nLayers = ffnn->nLayers;
    r32 radius = 10;
    r32 padding = 30;
    for(int layerIdx = 0;
            layerIdx < ffnn->nLayers;
            layerIdx++)
    {
        VecR32 *layer = ffnn->layers[layerIdx];
        NeuronLayer *neuronLayer = brain->layers+layerIdx;
        neuronLayer->nNeurons = layer->n;
        neuronLayer->neurons = PushArray(arena, Neuron, neuronLayer->nNeurons);
        neuronLayer->width = (radius+padding)*neuronLayer->nNeurons + 2*radius;
        for(int neuronIdx = 0;
                neuronIdx < neuronLayer->nNeurons;
                neuronIdx++)
        {
            Neuron *neuron = neuronLayer->neurons+neuronIdx;
            neuron->localPos = vec2(-neuronLayer->width/2+neuronIdx*(radius+padding),layerIdx*padding*2);
            neuron->radius = radius;
            neuron->tOffset = neuronIdx;
            neuron->xTRadius = RandomFloat(1, 8);
            neuron->yTRadius = RandomFloat(1, 8);
        }
    }
}

GuyBrain *
CreateGuyBrain(MemoryArena *arena, 
        int inputLayerSize, 
        int outputLayerSize, 
        int hiddenLayerSize, 
        int nHiddenLayers)
{
    GuyBrain *brain = PushStruct(arena, GuyBrain);
    brain->brain = CreateFFNN(arena, inputLayerSize, outputLayerSize, hiddenLayerSize, nHiddenLayers);
    InitGuyBrain(arena, brain);
    return brain;
}

void
DrawCloud(World *world, 
        Mesh *spriteBatch,
        Vec2 pos,
        int nParticles,
        r32 width,
        r32 height,
        Vec2 texOrig,
        Vec2 texSize)
{
    r32 time = world->time;
    Vec2 positions[nParticles];
    r32 radius[nParticles];
    r32 size = sqrtf(width*width + height*height)/2;
    for(int particleIdx = 0;
            particleIdx < nParticles;
            particleIdx++)
    {
        r32 r = size;
        radius[particleIdx] = r;
        positions[particleIdx] = vec2(pos.x+cosf(time+particleIdx*2)*width/2, 
                pos.y+sinf(time+particleIdx)*height/2);
        spriteBatch->colorState = vec4(0,0,0,1);
        PushCircle2(spriteBatch, positions[particleIdx], radius[particleIdx]+2, texOrig, texSize);
    }
    for(int particleIdx = 0;
            particleIdx < nParticles;
            particleIdx++)
    {
        spriteBatch->colorState = vec4(1,1,1,1);
        PushCircle2(spriteBatch, positions[particleIdx], radius[particleIdx], texOrig, texSize);
    }
}

void
DrawCloudLine(World *world, 
        Mesh *spriteBatch,
        Vec2 from,
        Vec2 to,
        int cloudPoints,
        Vec2 texOrig,
        Vec2 texSize)
{
    for(int atCloud = 0; 
            atCloud < cloudPoints;
            atCloud++)
    {
        r32 factor = (atCloud)/((r32)cloudPoints-1.0);
        Vec2 pos = Lerp2(from, to, factor);
#if 0
        pos.x+=atCloud*cosf(world->time*2);
        pos.y+=atCloud*sinf(world->time*5);
#endif
        r32 radius = (atCloud+1)*5;
        DrawCloud(world, spriteBatch, pos, 5, radius*2, radius*2, texOrig, texSize);
    }
}

void
DrawBrain(World *world, 
        GuyBrain *brain,
        Vec2 pos,
        Mesh *spriteBatch, 
        Vec2 circleTexPos, 
        Vec2 circleTexSize, 
        Vec2 rectTexPos, 
        Vec2 rectTexSize)
{
    r32 time = world->time;
    // Draw synapses
    spriteBatch->colorState = vec4(1.0, 0, 0, 1);
    for(int layerFromIdx = 0;
            layerFromIdx < brain->nLayers-1;
            layerFromIdx++)
    {
        NeuronLayer *layerFrom = brain->layers+layerFromIdx;
        NeuronLayer *layerTo = brain->layers+layerFromIdx+1;
        for(int neuronFromIdx = 0;
                neuronFromIdx < layerFrom->nNeurons;
                neuronFromIdx++)
        {
            Neuron *from = layerFrom->neurons+neuronFromIdx;
            for(int neuronToIdx = 0;
                    neuronToIdx < layerTo->nNeurons;
                    neuronToIdx++)
            {
                Neuron *to = layerTo->neurons+neuronToIdx;
                PushLine2(spriteBatch, v2_add(pos, from->pos), v2_add(pos, to->pos), 
                        2, rectTexPos, rectTexSize);
            }
        }
    }
    // TODO: updating positions here causes 1 frame of lag for drawing the connections.
    for(int layerIdx = 0;
            layerIdx < brain->nLayers;
            layerIdx++)
    {
        NeuronLayer *layer = brain->layers+layerIdx;
        for(int neuronIdx = 0;
                neuronIdx < layer->nNeurons;
                neuronIdx++)
        {
            Neuron *neuron = layer->neurons+neuronIdx;
            r32 tOffset = neuron->tOffset;
            r32 xr = neuron->xTRadius;
            r32 yr = neuron->yTRadius;
            Vec2 bPos = vec2(
                    neuron->localPos.x+xr*cosf(tOffset+time), 
                    neuron->localPos.y+yr*sinf(tOffset+time)
                    );
            neuron->pos = bPos;
            spriteBatch->colorState = vec4(0,0,0,1);
            PushCircle2(spriteBatch, v2_add(pos, bPos), neuron->radius+2, circleTexPos, circleTexSize);
            spriteBatch->colorState = vec4(1.0, 1, 0, 1);
            PushCircle2(spriteBatch, v2_add(pos, bPos), neuron->radius, circleTexPos, circleTexSize);
        }
    }
}
