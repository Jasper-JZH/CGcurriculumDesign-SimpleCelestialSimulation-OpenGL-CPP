#pragma once

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include"Shader.h"


class PointLight
{
public:
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    //下面这三个参数暂时不做调整
    const float constant = 1.0f;
    const float linear = 0.045f;
    const float quadratic = 0.0075f;


    unsigned int ID;
    PointLight(){}

    PointLight(unsigned int _ID, glm::vec3 _position, glm::vec3 _ambient = glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3 _diffuse = glm::vec3(0.9f, 0.9f, 0.9f), glm::vec3 _specular = glm::vec3(1.0f, 1.0f, 1.0f))
        :ID(_ID),position(_position), ambient(_ambient), diffuse(_diffuse),specular(_specular)
    { }

    PointLight(unsigned int _ID, const PointLight _light)
        :ID(_light.ID), position(_light.position), ambient(_light.ambient), diffuse(_light.diffuse), specular(_light.specular)
    { }

    //设置/装配光照到着色器中
    void Update(Shader shader)
    {
        std::string name = getStrName(ID);
        shader.setVec3(name + ".position", position);
        shader.setVec3(name + ".ambient", ambient);
        shader.setVec3(name + ".diffuse", diffuse);
        shader.setVec3(name + ".specular", specular);
        shader.setFloat(name + ".constant", constant);
        shader.setFloat(name + ".linear", linear);
        shader.setFloat(name + ".quadratic", quadratic);
    }
private:

    std::string getStrName(unsigned int _ID)
    {
        std::string temp = "pointLights[" + std::to_string(ID) + "]";
        return temp;
    }
};