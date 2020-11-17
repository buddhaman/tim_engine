#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
    vec4 texColor = texture(texture0, texCoord);
    FragColor = vec4(1.0, 1.0, 1.0, texColor.r);
}
