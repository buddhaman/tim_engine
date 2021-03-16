
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
};

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
VecR32Set(VecR32 *result, VecR32 *vec)
{
    Assert(result->n == vec->n);
    for(int i = 0; i < vec->n; i++)
    {
        result->v[i] = vec->v[i];
    }
}


void
VecR32MulS(VecR32 *vec, r32 s)
{
    for(int i = 0; i < vec->n; i++)
    {
        vec->v[i] = s*vec->v[i];
    }
}

void
VecR32AddS(VecR32 *vec, r32 s)
{
    for(int i = 0; i < vec->n; i++)
    {
        vec->v[i] = s+vec->v[i];
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
        r32 r = ((random() % 10000) / 10000.0)*(max-min)+min;
        vec->v[i] = r;
    }
}

void
RandomMatR32(MatR32 *mat, r32 min, r32 max)
{
    for(int y = 0; y < mat->h; y++)
    for(int x = 0; x < mat->w; x++)
    {
        // TODO: Better random
        r32 r = ((random() % 10000) / 10000.0)*(max-min)+min;
        MAT_VAL(mat, x, y) = r;
    }
}

void
RandomIntMatR32(MatR32 *mat, int min, int max)
{
    for(int y = 0; y < mat->h; y++)
    for(int x = 0; x < mat->w; x++)
    {
        // TODO: Better random
        r32 r = random()%(max-min)+min;
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

