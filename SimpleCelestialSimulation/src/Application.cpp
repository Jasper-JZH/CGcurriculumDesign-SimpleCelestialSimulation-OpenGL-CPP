#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "PointLight.h"
#include "Planet.h"

#include <iostream>
#include <string>
#include <list>
#include <vector>

#define glCheckError() glCheckError_(__FILE__, __LINE__) 

//函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> _facesPath);

// settings
extern const unsigned int SCR_WIDTH, SCR_HEIGHT;
const unsigned int SCR_WIDTH = 1300;
const unsigned int SCR_HEIGHT = 900;
float fov = (float)SCR_WIDTH / (float)SCR_HEIGHT;

// myCamera
Camera myCamera(glm::vec3(0.0f, 0.0f, 20.0f));
//Camera camera1(glm::vec3(0.0f, 2.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;


//立方体顶点数据
float cubeVertices[] = {
    // positions          // normals           // texture coords
    -1.0f, -1.0f,  1.0f,                        0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,                        1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,                        0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,                        1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,                        0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,                        1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,                        0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,                        1.0f, 1.0f
};

//立方体索引
unsigned int cubeIndices[] =
{
    0,1,3,  //正面
    3,2,0,
    1,5,7,  //右面
    7,3,1,
    4,0,2,  //左面
    2,6,4,
    5,4,6,  //后面
    6,7,5,
    2,3,7,  //上面
    7,6,2,
    4,5,1,  //下面
    1,0,4
};

//木箱子位置坐标
glm::vec3 cubePos[] = {
    glm::vec3(0.0f,  0.0f,  0.0f)
};

//点光源位置
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(0.0f,  0.0f,  0.0f)
};


//创建点光源
#define POINT_LIGHTS_NUM 2
//PointLight myPointLights [POINT_LIGHTS_NUM] = {};
std::vector<PointLight> myPointLights;


int main()
{
#pragma region glfw窗口初始化
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleCelestialSimulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Error:创建GLFW窗口失败！" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error:初始化GLAD失败！" << std::endl;
        return -1;
    }
#pragma endregion

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightShader("shader/lightVS.vert", "shader/lightFS.frag");
    Shader modelShader("shader/modelVS.vert", "shader/modelFS.frag");
    Shader skyBoxShader("shader/skyboxVS.vert", "shader/skyboxFS.frag");

    //行星
    Planet earth("earth", "model/earth/earth.obj", 5.0f, 10.0f, glm::vec3(0.0f, 1.0f, 0.0f), 2.0f, glm::vec3(0.25f, 0.25f, 0.25f));
    Planet sun("sun", "model/sun/sun.obj", 0.0f, 0.2f);
    Planet moon("moon", "model/moon/moon.obj", 0.5f, 0.3f, glm::vec3(0.0f, 1.0f, 0.0f), 20.0f, glm::vec3(0.08f, 0.08f, 0.08f));
  
    //设置立方体
    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);  //将数据传到显存

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);   //顶点坐标
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));   //纹理坐标
    glEnableVertexAttribArray(1);

    //设置天空盒的VAO（不需要纹理坐标）
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);  //将数据传到显存

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);   //顶点坐标
    glEnableVertexAttribArray(0);

    //加载天空盒材质
    vector<string> facesPath    //天空盒纹理路径
    {
        "texture/skybox/_left.png",
        "texture/skybox/_right.png",
        "texture/skybox/_top.png",
        "texture/skybox/_bottom.png",
        "texture/skybox/_front.png",
        "texture/skybox/_back.png"
    };

    unsigned int skyboxTexture = loadCubemap(facesPath);

    skyBoxShader.use();
    skyBoxShader.setInt("skybox", 0);


    //设置模型的部分材质
    modelShader.use();
    modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    modelShader.setFloat("material.shininess", 64.0f);
    
    //设置定向光
    modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    modelShader.setVec3("dirLight.color", 1.0f, 1.0f, 1.0f);

    //创建点光源
    for (int i = 0; i < POINT_LIGHTS_NUM; i++)
    {
        PointLight pointLight(i, pointLightPositions[i]);
        pointLight.Update(modelShader);
        myPointLights.push_back(pointLight);
    }


    //设置聚光灯
    modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

    modelShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    modelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    modelShader.setFloat("spotLight.constant", 1.0f);
    modelShader.setFloat("spotLight.linear", 0.09f);
    modelShader.setFloat("spotLight.quadratic", 0.032f);


    //渲染循环
    while (!glfwWindowShouldClose(window))
    {
        //计算每帧的时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glfwSetKeyCallback(window, key_callback);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //设置光源着色器相关的参数
        modelShader.use();
        modelShader.setVec3("viewPos", myCamera.Position);
        //聚光灯方向
        modelShader.setVec3("spotLight.position", myCamera.Position);
        modelShader.setVec3("spotLight.direction", myCamera.Front);


        ///绘制light(lamp)
        lightShader.use();  //设置mvp等参数前记得激活Shader程序！
        //设置v矩阵
        glm::mat4 view = myCamera.GetViewMatrix();
        lightShader.setMat4("view", view);
        //设置p矩阵
        glm::mat4 projection = glm::perspective(glm::radians(myCamera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        lightShader.setMat4("projection", projection);
        //设置m矩阵
        for (int i = 0; i < POINT_LIGHTS_NUM; i++)
        {
            glm::mat4 lightModel = glm::mat4(1.0f);  //记得要这样初始化矩阵
            lightModel = glm::translate(lightModel, pointLightPositions[i]);
            lightModel = glm::scale(lightModel, glm::vec3(0.2f));
            lightShader.setMat4("model", lightModel);

            glBindVertexArray(cubeVAO); //设置当前要绘制的VAO并调用Draw绘制
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        ///绘制行星

        //更新v,p矩阵后面可以整合到一起
        earth.Update_V_P(myCamera, fov);
        sun.Update_V_P(myCamera, fov);
        moon.Update_V_P(myCamera, fov);

        ///太阳
        sun.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        sun.Rotate(deltaTime);
        sun.Update_M();
        
        sun.SetMVP(lightShader);
        sun.Draw(lightShader);

        ///地球
        earth.RotateAround(sun.position, deltaTime);
        earth.UpdataRotateAxis_s();
        earth.Rotate(deltaTime);
        earth.Update_M();
        
        earth.SetMVP(modelShader);
        earth.Draw(modelShader);

        ///月球
        moon.RotateAround(earth.position, deltaTime);
        moon.Rotate(deltaTime);
        moon.Update_M();

        moon.SetMVP(modelShader);
        moon.Draw(modelShader);

        ///绘制天空盒
        glDepthFunc(GL_LEQUAL); //确保天空盒的深度值在小于或等于深度缓冲时通过测试
        skyBoxShader.use();
        view = glm::mat4(glm::mat3(myCamera.GetViewMatrix())); //移除位移
        skyBoxShader.setMat4("view", view);
        skyBoxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);  
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture); //skyboxTexture绑在0号纹理

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS); // 恢复正常的深度测试

        //交换帧缓冲并轮询键盘/鼠标事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 可选：销毁资源
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &cubeVBO);

    glfwTerminate();
    return 0;
}

//处理键盘输入事件1
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        myCamera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        myCamera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        myCamera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        myCamera.ProcessKeyboard(RIGHT, deltaTime);

}

//处理键盘输入事件2——只在按下去的瞬间检测一次
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //tab键切换相机模式
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        myCamera.ChangeMode();
        std::cout << "change Camera mode" << std::endl;
    }
        
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    myCamera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    myCamera.ProcessMouseScroll(yoffset);
}


//加载立方体纹理（天空盒）
unsigned int loadCubemap(vector<std::string> _facesPath)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < _facesPath.size(); i++)
    {
        unsigned char* data = stbi_load(_facesPath[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << _facesPath[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

