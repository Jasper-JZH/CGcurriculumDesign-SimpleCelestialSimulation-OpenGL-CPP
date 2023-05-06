#version 330 core

out vec4 FragColor;
in vec2 TexCoords;  //漫反射(纹理)贴图

uniform sampler2D diffuse;

void main()
{
	FragColor = texture(diffuse,TexCoords);
	//vec4 sunColor = vec4(0.95,0.55,0.0,1.0);
	//FragColor = sunColor * texture(diffuse,TexCoords);
	//FragColor = vec4(1.0);
}