#ifndef TIM_TYPES
#define TIM_TYPES

#define DebugOut(args...) printf("%20s%5d: ", __FILE__, __LINE__); printf(args); printf("\n");
#define Assert(expr) if(!(expr)) {DebugOut("assert failed "#expr""); \
    *((int *)0)=0;}

#define local_persist static
#define internal static
#define global_variable static

typedef unsigned char ui8;
typedef unsigned short ui16;
typedef unsigned int ui32;
typedef unsigned long ui64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;

typedef float r32;
typedef double r64;

typedef i32 b32;

#endif
