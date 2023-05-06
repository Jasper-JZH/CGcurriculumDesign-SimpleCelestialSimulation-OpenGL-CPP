#version 330 core

#define NR_POINT_LIGHTS 2   //使用预处理指令定义场景中点光源的数量

//定义模型的材质
struct Material //结构体布局(layout)
{
    sampler2D diffuse;  //使用漫反射贴图替代diffuse
    vec3 specular; 
    float shininess;
};

//定向光结构体
struct DirLight {
    vec3 direction;
    vec3 color;
}; 

//点光源结构体
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;

    float constant;
    float linear;
    float quadratic;
};  

//聚光灯结构体
struct SpotLight
{   
    vec3 position;      
    vec3 direction;
    vec3 color;

    //光强衰弱计算参数
    float constant;
    float linear;
    float quadratic;

    //聚光灯范围衰的cos
    float cutOff;       //内圈
    float outerCutOff;  //外圈
};

//函数声明
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


//其他参数
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;  //漫反射贴图

out vec4 FragColor;

//uniform定义
uniform vec3 viewPos;   //相机（观察者）的位置
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

void main()
{
    // 属性
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    // 第一阶段：定向光照
    //result += CalcDirLight(dirLight, norm, viewDir);
    //第二阶段：逐一计算点光源
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result+=CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    //第三阶段：聚光灯
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);   

    FragColor = vec4(result, 1.0);
}


//计算定向光
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 合并结果
    vec3 ambient  = light.color * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.color * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.color * spec * material.specular;
    //定向光没有削弱，所以直接返回
    //return (ambient + diffuse + specular); 
    return (ambient + diffuse);     //无反射
}

//计算点光源
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // 计算光线方向
    vec3 lightDir = normalize(light.position - fragPos);    
    // 漫反射着色
    // 计算漫反射强度
    float diff = max(dot(normal, lightDir), 0.0);           
    // 镜面光着色
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 衰减计算
    // 计算片元与光源的距离
    float distance    = length(light.position - fragPos);   
    float attenuation = 1.0 / (light.constant + light.linear * distance +   //计算衰弱因子
                 light.quadratic * (distance * distance));    
    // 合并结果
    // 使用texture函数对漫反射贴图采样，环境光使用同样的贴图
    vec3 ambient  = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    //vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse ) * attenuation;   //返回衰弱后的光照结果
}

//计算聚光灯效果
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient  = light.color * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.color * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.color * spec * material.specular;
    return (ambient + diffuse + specular) * attenuation * intensity;
}