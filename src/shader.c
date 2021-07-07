
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

void
InitShader(MemoryArena *arena, Shader *shader, const char *vertexPath, const char *fragmentPath)
{
    shader->fragmentSourcePath = PushString(arena, fragmentPath);
    shader->vertexSourcePath = PushString(arena, vertexPath);
}

void
LoadShader(Shader *shader)
{
    shader->fragmentSource = ReadEntireFile(shader->fragmentSourcePath);
    shader->vertexSource = ReadEntireFile(shader->vertexSourcePath);

    //printf("Shader source for %s:\n %s\n", shader->fragmentSourcePath, shader->fragmentSource);

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

void
InitShaderInstance(ShaderInstance *instance, Shader *shader)
{
    *instance = (ShaderInstance){};
    instance->shader = shader;
}

void 
InitSpriteShaderInstance(ShaderInstance *instance,
        Shader *shader,
        Mat3 *transform)
{
    InitShaderInstance(instance, shader);

    instance->transformLocation = glGetUniformLocation(shader->program, "transform");
    instance->hasMat3Transform = 1;
    instance->mat3Transform = transform;
}

void
InitBlurShader(ShaderInstance *instance, 
        Shader *shader)
{
    InitShaderInstance(instance, shader);

    instance->transformLocation = glGetUniformLocation(shader->program, "transform");
    instance->hasMat3Transform = 1;

    instance->radiusLocation = glGetUniformLocation(shader->program, "radius");
    instance->hasRadius = 1;

    instance->dir2Location = glGetUniformLocation(shader->program, "dir2");
    instance->hasDir2 = 1;
}

void
BeginShaderInstance(ShaderInstance *shaderInstance)
{
    glUseProgram(shaderInstance->shader->program);

    if(shaderInstance->hasMat3Transform)
    {
        Assert(shaderInstance->mat3Transform);
        glUniformMatrix3fv(shaderInstance->transformLocation, 1, 0, (GLfloat *)shaderInstance->mat3Transform);
    }

    if(shaderInstance->hasMat4Transform)
    {
        Assert(shaderInstance->mat4Transform);
        glUniformMatrix3fv(shaderInstance->transformLocation, 1, 0, (GLfloat *)shaderInstance->mat4Transform);
    }

    if(shaderInstance->hasRadius)
    {
        glUniform1f(shaderInstance->radiusLocation, shaderInstance->radius);
    }

    if(shaderInstance->hasDir2)
    {
        glUniform2f(shaderInstance->dir2Location, shaderInstance->dir2.x, shaderInstance->dir2.y);
    }

}

