
typedef struct
{
    int i;
    char cool[128];
} TestStruct;

void 
PrintTestStruct(TestStruct *test)
{
    printf("i = %d, cool = %s\n", test->i , test->cool);
}

void
DoMemoryTests()
{
    printf("Starting memory test\n");
    MemoryArena *arena = CreateMemoryArena(1024 * 1024 * 1024);
    MemoryPool *pool = CreateMemoryPool(arena, sizeof(TestStruct), 4);
    PrintMemoryPool(pool);
    TestStruct *a = AllocateElement(pool);
    TestStruct *b = AllocateElement(pool);
    TestStruct *c = AllocateElement(pool);
    printf("\n");
    printf("a %p\n", a);
    printf("b %p\n", b);
    printf("c %p\n", c);

    int num = 60;
    TestStruct *array[num];
    memset(array, 0, num*sizeof(TestStruct*));
    for(int i = 0; i < num; i++)
    {
        array[i] = AllocateElement(pool);
    }
    for(int i = 0; i < num; i++)
    {
        array[i]->i = i;
        sprintf(array[i]->cool, "hello dit is %d", i);
    }

    FreeElement(pool, a);
    FreeElement(pool, c);
    // FreeElement(pool, array[20]);
    // FreeElement(pool, array[21]);

    for(int i = 0; i < num; i++)
    {
        PrintTestStruct(array[i]);
    }
    PrintMemoryPool(pool);
}

