
enum
{
    SV_INITIAL = 1,
    SV_LATEST_PLUS_ONE,
};

#define LATEST_VERSION (SV_LATEST_PLUS_ONE-1)

typedef struct
{
    U32 dataVersion;
    FILE *file;
    B32 isWriting;
} Serializer;


