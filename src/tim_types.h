#ifndef TIM_TYPES
#define TIM_TYPES

#define DEBUG_OUT_SHOW_LINE 1

#if DEBUG_OUT_SHOW_LINE==0
#define DebugOut(args...) printf("%20s%5d: ", __FILE__, __LINE__); printf(args); printf("\n")
#else
#define DebugOut(args...) printf(args); printf("\n")
#endif

#define Assert(expr) if(!(expr)) {DebugOut("%s %d: assert failed : %s",__FILE__, __LINE__, ""#expr""); \
    *((int *)0)=0;}

#define local_persist static
#define internal static
#define global_variable static

#define Min(a, b) (a) < (b) ? (a) : (b)
#define Max(a, b) (a) < (b) ? (b) : (a)
#define Clamp(min, value, max) (value) < (min) ? (min) : ((value) > (max) ? (max) : (value))

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

typedef float R32;
typedef double R64;

typedef I32 B32;

#endif
