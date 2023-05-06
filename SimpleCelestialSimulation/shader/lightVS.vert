#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;//法线数据
layout (location = 2) in vec2 aTexCoords;//漫反射贴图的纹理坐标

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoords = aTexCoords; 
}