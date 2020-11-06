#version 330 core

in vec3 textureCoords;
in vec3 normal;
in vec3 pos;

out vec4 FragColor;
uniform sampler2D texture0;

void main()
{
    vec3 color = texture(texture0, pos.xy).xyz;
    vec3 lightDir = vec3(-1, -1, -1);
    lightDir = normalize(lightDir);
    float dp = max(-dot(normal, lightDir), 0);
    float diffuse = 0.4;
    float lightFactor = diffuse * (1.0-dp) + dp;
    //lightFactor = 1.0;
    //FragColor = vec4(lightFactor*color, 1.0);
    FragColor = vec4(color, 1.0);
}

