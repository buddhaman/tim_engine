
static int SEED = 0;

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40, 185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204, 9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81, 70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13, 203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41, 164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105, 228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89, 232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174, 193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145, 101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167, 135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255, 114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y)
{
    int tmp = hash[(y + SEED) % 256];
    return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y)
{
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth)
{
    float xa = x*freq;
    float ya = y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

ui32
CreateAndCompileShaderSource(char *source, GLenum shaderType)
{
    const char *const* src = (const char *const *)&source;
    ui32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, src, NULL);
    glCompileShader(shader);
    i32 succes;
    char infolog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &succes);
    if(!succes)
    {
        glGetShaderInfoLog(shader, 512, NULL, infolog);
        printf("%s\n", infolog);
        glDeleteShader(shader);
    }
    else
    {
        DebugOut("Shader %u compiled succesfully.", shader);
    }
    return shader;
}

ui32 
CreateAndLinkShaderProgram(ui32 vertexShader, ui32 fragmentShader)
{
    ui32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    i32 succes;
    char infolog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &succes);
    if(!succes)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        DebugOut("%s", infolog);
        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
    }
    else
    {
        DebugOut("Shader program %u linked succesfully. Cleaning up.", shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    return shaderProgram;
}

//TODO: store in own memory
void
InitShader(Shader *shader, const char *vertexPath, const char *fragmentPath)
{
    shader->fragmentSourcePath = malloc(sizeof(strlen(fragmentPath)));
    shader->vertexSourcePath = malloc(sizeof(strlen(vertexPath)));
    strcpy(shader->fragmentSourcePath, fragmentPath);
    strcpy(shader->vertexSourcePath, vertexPath);
}

void
LoadShader(Shader *shader)
{
    shader->fragmentSource = ReadEntireFile(shader->fragmentSourcePath);
    shader->vertexSource = ReadEntireFile(shader->vertexSourcePath);

    ui32 fragmentShader = CreateAndCompileShaderSource(shader->fragmentSource, GL_FRAGMENT_SHADER);
    ui32 vertexShader = CreateAndCompileShaderSource(shader->vertexSource, GL_VERTEX_SHADER);
    shader->program = CreateAndLinkShaderProgram(fragmentShader, vertexShader);
}

void
UnloadShader(Shader *shader)
{
    glDeleteProgram(shader->program);
    free(shader->fragmentSource);
    free(shader->vertexSource);
}

internal inline int
GetAttributeSize(VertexAttributeFlag attr)
{
    switch(attr)
    {
    case ATTR_POS2: { return 2; } break;
    case ATTR_POS3: { return 3; } break;
    case ATTR_COL4: { return 4; } break;
    case ATTR_TEX: { return 2; } break;
    case ATTR_NORM3: { return 3; } break;
    default: { return 0; } break;
    }
}

internal inline int
GetStride(ui32 attributes)
{
    ui32 attrValue = 1;
    int stride = 0;
    for(int i = 0; i < N_ATTRIBUTES; i++)
    {
        stride+=GetAttributeSize(attrValue & attributes);
        attrValue<<=1;
    }
    return stride;
}

internal inline Vec4
ARGBToVec4(ui32 hex)
{
    return vec4( (hex >> 24 & 255)/255.0f,
            (hex >> 16 & 255)/255.0f,
            (hex >> 8 & 255)/255.0f,
            (hex & 255)/255.0f);
}

internal void
InitCamera(Camera *camera)
{
    camera->spherical = vec3(3*M_PI/2.0, 1, 30);
}

internal inline Vec2
GetScreenPos(Camera *camera, Vec3 worldPos, r32 width, r32 height)
{
    Vec3 pos = m4_mul_pos(camera->transform, worldPos);
    return vec2((1+pos.x)*width/2.0, height-(1+pos.y)*height/2.0);
}

internal void
UpdateCamera(Camera *camera, r32 width, r32 height)
{
    r32 theta = camera->spherical.x;
    r32 phi = camera->spherical.y;
    r32 r = camera->spherical.z;
    camera->pos = vec3(
            cosf(theta) * cosf(phi) * r + camera->lookAt.x,
            sinf(theta) * cosf(phi) * r + camera->lookAt.y,
            sinf(phi) * r + camera->lookAt.z
            );
    Mat4 transform = m4_look_at(camera->pos, camera->lookAt, vec3(0,0,1));
    camera->transform = m4_mul(
            m4_perspective(90, width/height, 1, 1000),
            transform
            );
}

internal void
InitMesh(MemoryArena *arena, Mesh *mesh, ui32 vertexAttributes, int maxVertices)
{
    *mesh = (Mesh){};
    mesh->colorState = vec4(1,1,1,1);
    mesh->nVertices = 0;
    mesh->vertexAttributes = vertexAttributes;

    mesh->maxVertices = maxVertices;
    if(vertexAttributes & ATTR_POS2) mesh->vertices2 = PushArray(arena, Vec2, maxVertices);
    if(vertexAttributes & ATTR_POS3) mesh->vertices = PushArray(arena, Vec3, maxVertices);
    if(vertexAttributes & ATTR_COL4) mesh->colors = PushArray(arena, Vec4, maxVertices);
    if(vertexAttributes & ATTR_NORM3) mesh->normals = PushArray(arena, Vec3, maxVertices);
    if(vertexAttributes & ATTR_TEX) mesh->texCoords = PushArray(arena, Vec2, maxVertices);

    mesh->nIndices = 0;
    mesh->maxIndices = maxVertices;
    mesh->indices = PushArray(arena, ui32, maxVertices);
}

internal Mesh *
CreateMesh(MemoryArena *arena, ui32 vertexAttributes, int maxVertices)
{
    Mesh *mesh = PushStruct(arena, Mesh);
    InitMesh(arena, mesh, vertexAttributes, maxVertices);
    return mesh;
}

internal void
ClearMesh(Mesh *mesh)
{
    mesh->nVertices = 0;
    mesh->nIndices = 0;
}

internal void
PrintAttributes(Model *model)
{
    for(int aIdx = 0;
            aIdx < model->nVertexAttributes;
            aIdx++)
    {
        VertexAttribute *attr = model->vertexAttributes+aIdx;
        DebugOut("flag %u, offset %d", attr->type, attr->offset);
    }
}

internal void
InitModel(MemoryArena *arena, Model *model, ui32 vertexAttributes, int maxVertices)
{
    glGenVertexArrays(1, &model->vao);
    glGenBuffers(1, &model->vbo);
    glGenBuffers(1, &model->ebo);

    model->stride = GetStride(vertexAttributes);
    model->vertexBufferSize = 0;
    model->maxVertexBufferSize = maxVertices*model->stride;
    model->maxIndexBufferSize = maxVertices;
    model->indexBufferSize = 0;

    model->vertexBuffer = PushArray(arena, r32, maxVertices*model->stride);
    model->indexBuffer = PushArray(arena, ui32, maxVertices);

    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);

    DebugOut("Now making a model with stride %d", model->stride);
    ui32 attribute = 1;
    model->nVertexAttributes = 0;
    int offset = 0;
    size_t memOffset = 0;
    for(int attrIdx = 0; 
            attrIdx < N_ATTRIBUTES; 
            attrIdx++)
    {
        if(attribute & vertexAttributes)
        {
            int attributeSize = GetAttributeSize(attribute);

            DebugOut("Enable attribute %u with size %d", attribute, attributeSize);
            glEnableVertexAttribArray(model->nVertexAttributes);       
            glVertexAttribPointer(model->nVertexAttributes, attributeSize, GL_FLOAT, GL_FALSE, 
                    model->stride*sizeof(r32), (void*)memOffset);

            VertexAttribute *attr = model->vertexAttributes+model->nVertexAttributes;
            attr->type = attribute;
            attr->offset = offset;

            model->nVertexAttributes++;
            offset+=attributeSize;
            memOffset=offset*sizeof(r32);
        }
        attribute<<=1;
    }
    PrintAttributes(model);
#if 0
    glEnableVertexAttribArray(0);       // Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
            model->stride*sizeof(r32), (void *)0);
    glEnableVertexAttribArray(1);       // Colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
            model->stride*sizeof(r32), (void *)(3*sizeof(r32)));
    glEnableVertexAttribArray(2);       // Normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 
            model->stride*sizeof(r32), (void *)(6*sizeof(r32)));
#endif
}

internal void
SetVertexBufferValuesFromVec2(size_t nVertices, 
        size_t offset, 
        size_t stride,
        r32 dest[nVertices*stride],
        Vec2 src[nVertices])
{
    for(int idx = 0;
            idx < nVertices;
            idx++)
    {
        dest[idx*stride+offset] = src[idx].x;
        dest[idx*stride+offset+1] = src[idx].y;
    }
}

internal void
SetVertexBufferValuesFromVec3(size_t nVertices, 
        size_t offset, 
        size_t stride,
        r32 dest[nVertices*stride],
        Vec3 src[nVertices])
{
    for(int idx = 0;
            idx < nVertices;
            idx++)
    {
        dest[idx*stride+offset] = src[idx].x;
        dest[idx*stride+offset+1] = src[idx].y;
        dest[idx*stride+offset+2] = src[idx].z;
    }
}

internal void
SetVertexBufferValuesFromVec4(size_t nVertices, 
        size_t offset, 
        size_t stride,
        r32 dest[nVertices*stride],
        Vec4 src[nVertices])
{
    for(int idx = 0;
            idx < nVertices;
            idx++)
    {
        dest[idx*stride+offset] = src[idx].x;
        dest[idx*stride+offset+1] = src[idx].y;
        dest[idx*stride+offset+2] = src[idx].z;
        dest[idx*stride+offset+3] = src[idx].w;
    }
}


internal void
SetModelFromMesh(Model *model, Mesh *mesh, GLenum drawMode)
{
    model->vertexBufferSize = model->stride*mesh->nVertices;
    model->indexBufferSize = mesh->nIndices;

    // InterlaceVerticesColored(model, mesh);
    // Go over every attribute and place it into the vertex buffer

    for(int vertexAttributeIdx = 0;
            vertexAttributeIdx < model->nVertexAttributes;
            vertexAttributeIdx++)
    {
        VertexAttribute attribute = model->vertexAttributes[vertexAttributeIdx];
        size_t offset = attribute.offset;
        size_t stride = model->stride;
        size_t nVertices = mesh->nVertices;
        switch(attribute.type)
        {

        case ATTR_POS2:
        {
            SetVertexBufferValuesFromVec2(nVertices, 
                    offset, 
                    stride, 
                    model->vertexBuffer,
                    mesh->vertices2);
        } break;

        case ATTR_POS3:
        {
            SetVertexBufferValuesFromVec3(nVertices, 
                    offset, 
                    stride, 
                    model->vertexBuffer,
                    mesh->vertices);
        } break;

        case ATTR_TEX:
        {
            SetVertexBufferValuesFromVec2(nVertices, 
                    offset, 
                    stride, 
                    model->vertexBuffer,
                    mesh->texCoords);
        } break;

        case ATTR_COL4:
        {
            SetVertexBufferValuesFromVec4(nVertices, 
                    offset, 
                    stride, 
                    model->vertexBuffer,
                    mesh->colors);
        } break;

        case ATTR_NORM3:
        {
            SetVertexBufferValuesFromVec3(nVertices, 
                    offset,
                    stride, 
                    model->vertexBuffer,
                    mesh->normals);
        } break;

        default:
        {
            DebugOut("Attribute with flag %d not implemented", attribute.type);
        } break;

        }
    }
    Assert(model->maxIndexBufferSize > mesh->nIndices);

    memcpy(model->indexBuffer, mesh->indices, mesh->nIndices*sizeof(ui32));

    glBindVertexArray(model->vao);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
    glBufferData(GL_ARRAY_BUFFER, model->vertexBufferSize*sizeof(r32), 
            model->vertexBuffer, drawMode);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indexBufferSize*sizeof(ui32), 
            model->indexBuffer, drawMode);
}

internal inline void
PushVertex(Mesh *mesh, Vec3 pos, Vec3 normal, Vec2 texCoord)
{
    mesh->vertices[mesh->nVertices] = pos;
    mesh->colors[mesh->nVertices] = mesh->colorState;
    mesh->normals[mesh->nVertices] = normal;
    mesh->texCoords[mesh->nVertices] = texCoord;
    mesh->nVertices++;
    Assert(mesh->nVertices < mesh->maxIndices);
}

internal inline void 
PushIndex(Mesh *mesh, ui32 index)
{
    mesh->indices[mesh->nIndices++] = index;
    Assert(mesh->nIndices < mesh->maxIndices);
}

internal void
PushTriangle(Mesh *mesh, Vec3 p0, Vec3 p1, Vec3 p2, Vec2 u0, Vec2 u1, Vec2 u2)
{
    Vec3 diff0 = v3_sub(p1, p0);
    Vec3 diff1 = v3_sub(p2, p0);
    Vec3 normal = v3_norm(v3_cross(diff0, diff1));
    ui32 nVertices = mesh->nVertices;
    PushVertex(mesh, p0, normal, u0);
    PushVertex(mesh, p1, normal, u1);
    PushVertex(mesh, p2, normal, u2);
    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
}

// Assume in same plane
internal void
PushQuad(Mesh *mesh, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Vec2 texCoord, Vec2 texSize)
{
    Vec3 diff0 = v3_sub(p1, p0);
    Vec3 diff1 = v3_sub(p2, p0);
    Vec3 normal = v3_norm(v3_cross(diff0, diff1));
    ui32 nVertices = mesh->nVertices;
    PushVertex(mesh, p0, normal, texCoord);
    PushVertex(mesh, p1, normal, vec2(texCoord.x+texSize.x, texCoord.y));
    PushVertex(mesh, p2, normal, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
    PushVertex(mesh, p3, normal, vec2(texCoord.x, texCoord.y+texSize.y));
    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);
}

// Assume in same plane
internal void
PushDoubleSidedQuad(Mesh *mesh, Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3, Vec2 texCoord, Vec2 texSize)
{
    Vec3 diff0 = v3_sub(p1, p0);
    Vec3 diff1 = v3_sub(p2, p0);
    Vec3 normal = v3_norm(v3_cross(diff0, diff1));
    ui32 nVertices = mesh->nVertices;
    Vec3 mNormal = v3_muls(normal, -1);

    PushVertex(mesh, p0, normal, texCoord);
    PushVertex(mesh, p1, normal, vec2(texCoord.x+texSize.x, texCoord.y));
    PushVertex(mesh, p2, normal, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
    PushVertex(mesh, p3, normal, vec2(texCoord.x, texCoord.y+texSize.y));

    PushVertex(mesh, p3, mNormal, texCoord);
    PushVertex(mesh, p2, mNormal, vec2(texCoord.x+texSize.x, texCoord.y));
    PushVertex(mesh, p1, mNormal, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
    PushVertex(mesh, p0, mNormal, vec2(texCoord.x, texCoord.y+texSize.y));

    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);

    nVertices+=4;
    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);
}


// Assume normal and up are normalized. Also double sided.
internal void
PushRect3(Mesh *mesh, Vec3 center, Vec2 size, Vec3 normal, Vec3 up, Vec2 texCoord, Vec2 texSize)
{
    Vec3 vX = v3_muls(v3_norm(v3_cross(normal, up)), size.x/2.0);
    Vec3 vY = v3_muls(v3_norm(v3_cross(vX, normal)), size.y/2.0);
    Vec3 p0 = v3_sub(v3_sub(center, vY), vX);
    Vec3 p1 = v3_add(v3_sub(center, vY), vX);
    Vec3 p2 = v3_add(v3_add(center, vY), vX);
    Vec3 p3 = v3_sub(v3_add(center, vY), vX);
    PushDoubleSidedQuad(mesh, p0, p1, p2, p3, texCoord, texSize);
}

// Double sided
internal inline void
PushTrapezoid(Mesh *mesh, 
        Vec3 from, 
        Vec3 to, 
        r32 fromWidth, 
        r32 toWidth, 
        Vec3 normal, 
        Vec2 texCoord, 
        Vec2 texSize)
{
    Vec3 diff = v3_sub(to, from);
    Vec3 perp = v3_cross(diff, normal);
    perp = v3_norm(perp);
    Vec3 perp0 = v3_muls(perp, fromWidth/2);
    Vec3 perp1 = v3_muls(perp, toWidth/2);
    ui32 nVertices = mesh->nVertices;

    PushVertex(mesh, v3_add(from, v3_muls(perp0,-1)), normal, texCoord);
    PushVertex(mesh, v3_add(from, perp0), normal, vec2(texCoord.x+texSize.x, texCoord.y));
    PushVertex(mesh, v3_add(to, perp1), normal, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
    PushVertex(mesh, v3_add(to, v3_muls(perp1, -1)), normal, vec2(texCoord.x, texCoord.y+texSize.y));

    Vec3 mNormal = v3_muls(normal, -1);
    PushVertex(mesh, v3_add(to, v3_muls(perp1, -1)), mNormal, vec2(texCoord.x, texCoord.y+texSize.y));
    PushVertex(mesh, v3_add(to, perp1), mNormal, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
    PushVertex(mesh, v3_add(from, perp0), mNormal, vec2(texCoord.x+texSize.x, texCoord.y));
    PushVertex(mesh, v3_add(from, v3_muls(perp0,-1)), mNormal, texCoord);

    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);

    nVertices+=4;
    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);
}

internal inline void 
PushLine(Mesh *mesh, Vec3 from, Vec3 to,  r32 width, Vec3 normal, Vec2 texCoord, Vec2 texSize)
{
    PushTrapezoid(mesh, from, to, width, width, normal, texCoord, texSize);
}

internal inline void
PushLineCircle(Mesh *mesh, Vec3 center, r32 radius, int nPoints, r32 lineWidth, Vec2 texCoord, Vec2 texSize)
{
    Vec3 prev = vec3(center.x+radius, center.y, center.z);
    for(int point = 0;
            point < nPoints;
            point++)
    {
        r32 angle = point * (M_PI*2.0 / (nPoints-1));
        r32 c = cosf(angle);
        r32 s = sinf(angle);
        Vec3 p = v3_add(center, vec3(c*radius, s*radius, 0));
        PushLine(mesh, prev, p, lineWidth, vec3(0,0,1), texCoord, texSize);
        prev = p;
    }
}

internal r32
RandomFloat(r32 min, r32 max)
{
    return min + (max-min)*(rand()%10000)/10000.0;
}

internal inline Vec3
Lerp3(Vec3 from, Vec3 to, r32 lambda)
{
    return v3_add(v3_muls(from, 1.0-lambda), v3_muls(to, lambda));
}

void
PushHeightField(Mesh *mesh, r32 tileSize, int width, int height, Vec2 texCoord, Vec2 texSize)
{
    local_persist r32 xOffset = 0;
    local_persist r32 yOffset = 0;
    xOffset+=0.05;
    yOffset+=0.07;
    r32 xScale = 0.125;
    r32 yScale = 0.125;
    Vec3 positions[(width)*(height)];
    mesh->colorState = ARGBToVec4(0xffffffff);
    r32 depth = 3;
    for(int y = 0; y < height; y++)
    for(int x = 0; x < width; x++)
    {
        r32 perlin = perlin2d(x*xScale+xOffset, y*yScale+yOffset, 1, 7);
        r32 z = depth*perlin - depth;
        r32 dev = 0.3;
        positions[x+y*width] = vec3(tileSize*(x+RandomFloat(-dev, dev)),
                    tileSize*(y+RandomFloat(-dev, dev)), 
                    z);
    }
    for(int y = 0; y < height-1; y++)
    for(int x = 0; x < width-1; x++)
    {
        Vec3 p0 = positions[x+y*width];
        Vec3 p1 = positions[x+1+y*width];
        Vec3 p2 = positions[x+1+(y+1)*width];
        Vec3 p3 = positions[x+(y+1)*width];
        PushTriangle(mesh, p0, p1, p2, texCoord, 
                vec2(texCoord.x + texSize.x, texCoord.y), vec2(texCoord.x+texSize.x, texCoord.y+texSize.y));
        PushTriangle(mesh, p2, p3, p0, vec2(texCoord.x+texSize.x, texCoord.y+texSize.y),
                vec2(texCoord.x, texCoord.y+texSize.y), texCoord);
    }
}

internal inline void
PushCube(Mesh *mesh, Vec3 origin, Vec3 dimensions, Vec2 texCoord, Vec2 texSize)
{
    r32 x = dimensions.x;
    r32 y = dimensions.y;
    r32 z = dimensions.z;
    Vec3 p000 = origin;
    Vec3 p001 = v3_add(origin, vec3(0, 0, z));
    Vec3 p010 = v3_add(origin, vec3(0, y, 0));
    Vec3 p011 = v3_add(origin, vec3(0, y, z));
    Vec3 p100 = v3_add(origin, vec3(x, 0, 0));
    Vec3 p101 = v3_add(origin, vec3(x, 0, z));
    Vec3 p110 = v3_add(origin, vec3(x, y, 0));
    Vec3 p111 = v3_add(origin, vec3(x, y, z));
    PushQuad(mesh, p010, p110, p100, p000, texCoord, texSize);
    PushQuad(mesh, p001, p011, p010, p000, texCoord, texSize);
    PushQuad(mesh, p001, p101, p111, p011, texCoord, texSize);
    PushQuad(mesh, p100, p110, p111, p101, texCoord, texSize);
    PushQuad(mesh, p000, p100, p101, p001, texCoord, texSize);
    PushQuad(mesh, p011, p111, p110, p010, texCoord, texSize);
}

internal void
PushLineArrow(Mesh *mesh, Vec3 from, Vec3 to, Vec3 norm, Vec2 texCoord, Vec2 texSize, r32 lineWidth)
{
    Vec3 diff = v3_norm((v3_sub(to, from)));
    Vec3 perp = v3_cross(diff, norm);           
    r32 angle = -M_PI/2 + 0.5;
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    Vec3 head0 = v3_add(to, v3_add(v3_muls(perp, c), v3_muls(diff, s)));
    Vec3 head1 = v3_add(to, v3_add(v3_muls(perp, -c), v3_muls(diff, s)));
    PushLine(mesh, from, to, lineWidth, norm, texCoord, texSize);
    PushLine(mesh, to, head0, lineWidth, norm, texCoord, texSize);
    PushLine(mesh, to, head1, lineWidth, norm, texCoord, texSize);
}

internal void
RenderModel(Model *model)
{
    glBindVertexArray(model->vao);
    glDrawElements(GL_TRIANGLES, model->indexBufferSize, GL_UNSIGNED_INT, 0);
}


