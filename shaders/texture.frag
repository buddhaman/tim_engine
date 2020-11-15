#version 330 core

in vec2 textureCoords;
in vec3 normal;

out vec4 FragColor;
uniform sampler2D texture0;

void main()
{
    float diffuse = 0.4;
    vec3 texColor = texture(texture0, textureCoords).xyz;
    vec3 color = texColor + vec3(diffuse, diffuse, diffuse);
    vec3 lightDir = vec3(-1, -1, -1);
    lightDir = normalize(lightDir);
    float dp = max(-dot(normal, lightDir), 0);
    float lightFactor = diffuse * (1.0-dp) + dp;
    //lightFactor = 1.0;
    FragColor = vec4(lightFactor*color, 1.0);
}

