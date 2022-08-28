#include <stdlib.h>

typedef struct 
{
    U8 *base;
    size_t size;
} GameMemory;

typedef struct
{
    U8 *base;
    size_t used;
    size_t size;
    GameMemory *memory;
} MemoryArena;

typedef struct
{
    U8 *base;
    size_t sizeInBytes;

    size_t elementSize;
    size_t maxBlocks;
    U32 *blocks;           // Blocks of size 32.
} MemoryPool;

