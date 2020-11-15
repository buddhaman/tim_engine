#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texCoord;

out vec2 texCoord;

uniform mat3 transform;

void main()
{
    texCoord = a_texCoord;
    gl_Position = vec4(transform * vec3(a_position, 1.0), 1.0);
}

