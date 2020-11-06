#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_texture;
layout (location = 2) in vec3 a_normal;

uniform mat4 transform;

out vec3 textureCoords;
out vec3 normal;
out vec3 pos;

void main()
{
    textureCoords = a_texture;
    normal = a_normal;
    pos = (transform * vec4(a_position, 1.0)).xyz;
    gl_Position = transform * vec4(a_position, 1.0);
}


