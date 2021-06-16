
size_t
SizeOfMatR32(int w, int h)
{
    return w*h*sizeof(r32)+sizeof(MatR32);
}

MatR32 *
CreateMatR32(int w, int h, void *memory)
{
    MatR32 *mat = memory;
    mat->m = memory+sizeof(MatR32);
    mat->w = w;
    mat->h = h;
    return mat;
}

internal inline void
InitMatR32FromGene(MatR32 *mat, int w, int h, VecR32 *gene, r32 **atMemory)
{
    mat->w = w;
    mat->h = h;
    mat->m = *atMemory;
    *atMemory += (w*h);
}

void
InitVecR32(VecR32 *vec, int n, void *memory)
{
    vec->n = n;
    vec->v = memory;
}

size_t
SizeOfVecR32(int n)
{
    return n*sizeof(r32)+sizeof(VecR32);
}

VecR32 *
CreateVecR32(int n, void *memory)
{
    VecR32 *vec = memory;
    vec->v = memory+sizeof(VecR32);
    vec->n = n;
    return vec;
};

internal inline void
InitVecR32FromGene(VecR32 *vec, int n, VecR32 *gene, r32 **atMemory)
{
    vec->n = n;
    vec->v = *atMemory;
    *atMemory += n;
}

#define MAT_VAL(mat, x, y) mat->m[x+y*mat->w]

void
MatR32ClearToZero(MatR32 *mat)
{
    memset(mat->m, 0, mat->w*mat->h*sizeof(r32));
}

void
MatR32SetToIdentity(MatR32 *mat)
{
    int d = mat->w < mat->h ? mat->w : mat->h;

    MatR32ClearToZero(mat);
    for(int i = 0; i < d; i++)
    {
        MAT_VAL(mat, i, i) = 1.0;
    }
}

// As column vector
void
MultiplyMatVecR32(VecR32 *result, MatR32 *mat, VecR32 *vec)
{
    for(int y = 0; y < mat->h; y++)
    {
        r32 dp = 0.0;
        for(int x = 0; x < mat->w; x++)
        {
            dp += vec->v[x]*MAT_VAL(mat, x, y);
        }
        result->v[y] = dp;
    }
}

void
VecR32Add(VecR32 *result, VecR32 *a, VecR32 *b)
{
    for(int i = 0; i < a->n; i++)
    {
        result->v[i] = a->v[i]+b->v[i];
    }
}

void
VecR32AddScaled(VecR32 *result, VecR32 *a, VecR32 *b, r32 factor)
{
    for(int i = 0; i < a->n; i++)
    {
        result->v[i] = a->v[i]+b->v[i]*factor;
    }
}

void
VecR32SetS(VecR32 *result,  r32 value)
{
    for(int i = 0; i < result->n; i++)
    {
        result->v[i] = value;
    }
}

void
VecR32Set(VecR32 *result, VecR32 *vec)
{
    Assert(result->n == vec->n);
    for(int i = 0; i < vec->n; i++)
    {
        result->v[i] = vec->v[i];
    }
}

// doesnt follow the normal convention of having a result vector for some reason.
// but whatever.
void
VecR32MulS(VecR32 *result, VecR32 *vec, r32 s)
{
    for(int i = 0; i < vec->n; i++)
    {
        result->v[i] = s*vec->v[i];
    }
}

void
VecR32AddS(VecR32 *result, VecR32 *vec, r32 s)
{
    for(int i = 0; i < vec->n; i++)
    {
        result->v[i] = s+vec->v[i];
    }
}

void 
VecR32Hadamard(VecR32 *result, VecR32 *a, VecR32 *b)
{
    Assert(a->n == b->n);
    for(int i = 0; i < a->n; i++)
    {
        result->v[i] = a->v[i]*b->v[i];
    }
}

void
VecR32SetNormal(VecR32 *vec, r32 dev)
{
    ui32 pairs = (vec->n)/2;

    for(ui32 atPair = 0;
            atPair < pairs;
            atPair++)
    {
        Vec2 v = RandomNormalPair();
        vec->v[atPair*2] = v.x*dev;
        vec->v[atPair*2+1] = v.y*dev;
    }
    if(vec->n%2==1)
    {
        vec->v[vec->n-1] = RandomNormalPair().x*dev;
    }
}

internal inline r32
VecR32Sum(VecR32 *vec)
{
    r32 sum = 0;
    for(ui32 idx = 0;
            idx < vec->n;
            idx++)
    {
        sum+=vec->v[idx];
    }
    return sum;
}

r32
VecR32Average(VecR32 *vec)
{
    r32 sum = VecR32Sum(vec);
    return sum/vec->n;
}

r32
VecR32Variance(VecR32 *vec)
{
    r32 avg = VecR32Average(vec);
    r32 sum = 0;
    for(int i = 0; i < vec->n; i++)
    {
        r32 diff = vec->v[i]-avg;
        sum+=(diff*diff);
    }
    sum/=vec->n;
    return sum;
}

r32
VecR32Length2(VecR32 *vec)
{
    r32 sum = 0;
    for(ui32 idx = 0;
            idx < vec->n;
            idx++)
    {
        sum+=vec->v[idx]*vec->v[idx];
    }
    return sum;
}

r32
VecR32Distance2(VecR32 *a, VecR32 *b)
{
    Assert(a->n == b->n);
    r32 sum = 0;
    for(ui32 idx = 0;
            idx < a->n;
            idx++)
    {
        r32 diff = b->v[idx] - a->v[idx];
        sum+=(diff*diff);
    }
    return sum;
}

#define VEC_TRANSFORM_FUNCTION(name) r32 name(r32 x)
typedef VEC_TRANSFORM_FUNCTION(vec_transform_function);

VEC_TRANSFORM_FUNCTION(Sigmoid)
{
    return 1.0/(1.0+expf(-x));
}

VEC_TRANSFORM_FUNCTION(HyperbolicTangent)
{
    return tanhf(x);
}

void
VecR32Apply(VecR32 *result, VecR32 *vec, vec_transform_function *f)
{
    for(int i = 0; i < vec->n; i++)
    {
        result->v[i] = f(vec->v[i]);
    }
}

void
RandomVecR32(VecR32 *vec, r32 min, r32 max)
{
    for(int i = 0; i < vec->n; i++)
    {
        r32 r = ((rand() % 10000) / 10000.0)*(max-min)+min;
        vec->v[i] = r;
    }
}

void
RandomMatR32(MatR32 *mat, r32 min, r32 max)
{
    for(int y = 0; y < mat->h; y++)
    for(int x = 0; x < mat->w; x++)
    {
        // TODO: Better rand
        r32 r = ((rand() % 10000) / 10000.0)*(max-min)+min;
        MAT_VAL(mat, x, y) = r;
    }
}

void
RandomIntMatR32(MatR32 *mat, int min, int max)
{
    for(int y = 0; y < mat->h; y++)
    for(int x = 0; x < mat->w; x++)
    {
        // TODO: Better rand
        r32 r = rand()%(max-min)+min;
        MAT_VAL(mat, x, y) = r;
    }
}

#undef MAT_VAL

// Printing

void
VecR32TransposePrintP(FILE *stream, VecR32 *v, int w, int p)
{
    for(int r = 0; r < v->n; r++)
    {
        fprintf(stream, "| ");
        fprintf(stream, "%*.*f ", w, p, v->v[r]);
        fprintf(stream, "|\n");
    }
}

void
VecR32TransposePrintF(VecR32 *v, int w, int p)
{
    VecR32TransposePrintP(stdout, v, w, p);
}

void
VecR32PrintP(FILE *stream, VecR32 *v, int w, int p)
{
    fprintf(stream, "| ");
    for(int r = 0; r < v->n; r++)
    {
        fprintf(stream, "%*.*f ", w, p, v->v[r]);
    }
    fprintf(stream, "|\n");
}

void
VecR32PrintF(VecR32 *v, int w, int p)
{
    VecR32PrintP(stdout, v, w, p);
}

void
MatR32PrintP(FILE *stream, MatR32 *m, int w, int p)
{
    for(int r = 0; r < m->h; r++)
    {
        fprintf(stream, "| ");
        for(int c = 0; c < m->w; c++)
        {
            fprintf(stream, "%*.*f ", w, p, m->m[r*m->w+c]);
        }
        fprintf(stream, "|\n");
    }
}

void
MatR32PrintF(MatR32 *m, int w, int p)
{
    MatR32PrintP(stdout, m, w, p);
}

