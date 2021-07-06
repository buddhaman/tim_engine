#version 330 core

in vec2 texCoord;
in vec4 color;

out vec4 FragColor;

uniform sampler2D texture0;

// TODO: make uniform
const float offset = 1.0/512.0;

void main()
{

    float kernel[9] = float[](
        1, 2, 1,
        2, 4, 2,
        1,  2, 1
    );
    vec4 col = vec4(0);
    float factor = 1.0/16.0;
    for(int y = 0; y < 3; y++)
    for(int x = 0; x < 3; x++)
    {
        float kernelValue = kernel[x+3*y];
        vec2 sampleAt = vec2((x-1)*offset, (y-1)*offset)+texCoord;
        col+=texture(texture0, sampleAt)*kernelValue;
    }
    col*=factor;

    FragColor = color*col;
}

