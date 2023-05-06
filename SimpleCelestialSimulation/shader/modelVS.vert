#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;//法线数据
layout (location = 2) in vec2 aTexCoords;//漫反射贴图的纹理坐标

out vec2 TexCoords;

out vec3 FragPos;//存放顶点在世界坐标中的位置，用于计算漫反射
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{   
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoords = aTexCoords; 
    FragPos = FragPos = vec3(model * vec4(aPos, 1.0));
    //使用法线矩阵"mat3(transpose(inverse(model)))"来计算法线
    Normal = mat3(transpose(inverse(model))) * aNormal;
}