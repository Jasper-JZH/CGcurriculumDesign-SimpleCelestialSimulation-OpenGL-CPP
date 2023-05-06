#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox; //天空盒材质

void main()
{    
    FragColor = texture(skybox, TexCoords);
}