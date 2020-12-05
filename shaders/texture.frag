#version 330 core

in vec2 textureCoords;
in vec3 normal;
in vec4 color;

out vec4 FragColor;
uniform sampler2D texture0;

void main()
{
    float diffuse = 0.4;
    vec4 texColorA = texture(texture0, textureCoords);
    if(texColorA.a < 0.1)
    {
        discard;
    }
    vec3 texColor = texColorA.rgb;
    vec3 col = color.xyz*texColor; 
    vec3 lightDir = vec3(-1, -1, -1);
    lightDir = normalize(lightDir);
    float dp = abs(-dot(normal, lightDir));
    float lightFactor = diffuse * (1.0-dp) + dp;
    //lightFactor = 1.0;
    FragColor = vec4(lightFactor*col, 1.0);
}

