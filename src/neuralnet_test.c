
void
DoNeuralNetTests()
{
    MemoryArena *arena = CreateMemoryArena(128*1024*1024);
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
}
