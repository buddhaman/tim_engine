
MemoryArena *
CreateMemoryArena(size_t sizeInBytes)
{
    ui8 *totalMemory = malloc(sizeof(MemoryArena)+sizeInBytes);
    MemoryArena *arena = (MemoryArena *)totalMemory;
    Assert(totalMemory);
    arena->base = totalMemory + sizeof(MemoryArena);
    arena->used = 0;
    arena->size = sizeInBytes;
    arena->memory = NULL;
    return arena;
}

void
ClearArena(MemoryArena *arena)
{
    memset(arena->base, 0, arena->used);
    arena->used = 0;
}

void *
PushMemory_(MemoryArena *arena, size_t size)
{
    arena->used+=size;
    Assert(arena->used < arena->size);
    return arena->base + arena->used - size;
}
#define PushStruct(arena, type) \
    (type *)PushMemory_(arena, sizeof(type)); \
    DebugOut(""#type" : %p", (void*)(arena->base+arena->used))
#define PushArray(arena, type, nElements) \
    (type *)PushMemory_(arena, sizeof(type)*nElements); \
    DebugOut(""#type" : %p", (void*)(arena->base+arena->used))

MemoryPool *
CreateMemoryPool(MemoryArena *arena, size_t elementSize, size_t maxBlocks)
{
    MemoryPool *pool = PushStruct(arena, MemoryPool);
    pool->elementSize = elementSize;
    pool->maxBlocks = maxBlocks;
    pool->sizeInBytes = elementSize*maxBlocks*32;

    // 32 is hardcoded for now
    pool->base = PushArray(arena, ui8, pool->sizeInBytes);
    pool->blocks = PushArray(arena, ui32, maxBlocks);
    memset(pool->base, 0, pool->sizeInBytes);
    memset(pool->blocks, 0, maxBlocks*sizeof(ui32));

    return pool;
}

void PrintMemoryBlock(ui32 block)
{
    for(int bit = 0;
            bit < 32;
            bit++)
    {
        b32 bit = block & (1<<31);
        putc(bit ? '1' : '0', stdout);
        block <<= 1;
    }
}

void *
AllocateElement(MemoryPool *pool)
{
    ui32 blockIdx = 0;
    ui32 bitIdx = 0;
    for(blockIdx = 0;
            blockIdx < pool->maxBlocks;
            blockIdx++)
    {
        ui32 block = pool->blocks[blockIdx];
        if((~(block))!=0U)
        {
            bitIdx = 0;
            while((block)&1) 
            {
                block>>=1;
                bitIdx++;
            }
            pool->blocks[blockIdx]|=(1<<bitIdx);
            break;
        }
    }
    DebugOut("allocate on block %u bit %u", blockIdx, bitIdx);
    return pool->base + (pool->elementSize*32)*blockIdx + pool->elementSize*bitIdx;
}

void
FreeElement(MemoryPool *pool, void *element)
{
    size_t elementByteOffset = ((size_t)element) - ((size_t)pool->base);
    size_t elementIdx = elementByteOffset/pool->elementSize;
    size_t blockIdx = elementIdx/32;
    size_t bitIdx = elementIdx-blockIdx*32;
    Assert(blockIdx < pool->maxBlocks);
    Assert(bitIdx < 32);
    Assert(pool->blocks[blockIdx]&(1<<bitIdx));
    pool->blocks[blockIdx]&=~(1<<bitIdx);
}

void
PrintMemoryPool(MemoryPool *pool)
{
    for(int blockIdx = 0;
            blockIdx < pool->maxBlocks;
            blockIdx++)
    {
        ui32 block = pool->blocks[blockIdx];
        printf("block %5d : ", blockIdx);
        PrintMemoryBlock(block);
        putc('\n', stdout);
    }
}

