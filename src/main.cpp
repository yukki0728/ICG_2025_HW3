#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "header/cube.h"
#include "header/Object.h"
#include "header/shader.h"
#include "header/stb_image.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateCamera();
void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta);
unsigned int loadCubemap(std::vector<std::string> &mFileName);

struct camera_t{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;

    float yaw;
    float pitch;
    float radius;
    float minRadius;
    float maxRadius;
    float orbitRotateSpeed;
    float orbitZoomSpeed;
    float minOrbitPitch;
    float maxOrbitPitch;
    bool enableAutoOrbit;
};

struct light_t{
    glm::vec3 position;
    glm::vec3 color;
};

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// cube map 
unsigned int cubemapTexture;
unsigned int cubemapVAO, cubemapVBO;

// shader programs 
int shaderProgramIndex = 0;
std::vector<shader_program_t*> shaderPrograms;
shader_program_t* cubemapShader;

light_t light;
camera_t camera;

//model
Object* dogModel = nullptr;
Object* soapModel = nullptr;
Object* dryerModel = nullptr;
Object* waterModel = nullptr;

float currentTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//soap
bool SoapShow = false;
float SoapStartTime = 0.0f;
float SoapShowTime = 5.0f;

//bubble
bool BubbleShow = false;
float BubbleAmount = 0.0f;
float BubbleGrowSpeed = 0.25f;

//dryer
bool DryerShow = false;
float DryerStartTime = 0.0f;
float DryerShowTime = 5.0f;

//water
bool WaterShow = false;
float WaterStartTime = 0.0f;
float WaterShowTime = 10.0f;

//fur
float FurStrength = 0.0f;
float minStrength = 0.0f;
float maxStrength = 12.5f;
float FurGrowSpeed = 2.5f;
float FurSmallSpeed = 1.0f;

//dog jump
bool DogJump = false;
float DogJumpStartTime = 0.0f;
float DogJumpDuration = 1.2f;

void model_setup(){
    dogModel = new Object("..\\..\\src\\asset\\model\\dog.obj");
    dogModel->loadTexture("..\\..\\src\\asset\\model\\dog_texture.png");

    soapModel = new Object("..\\..\\src\\asset\\model\\soap.obj");
    soapModel->loadTexture("..\\..\\src\\asset\\model\\soap_texture.png");

    dryerModel = new Object("..\\..\\src\\asset\\model\\dryer.obj");
    dryerModel->loadTexture("..\\..\\src\\asset\\model\\dryer_texture.png");

    waterModel = new Object("..\\..\\src\\asset\\model\\bubble.obj");
}

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 45.0f;
    camera.radius = 400.0f;
    camera.minRadius = 150.0f;
    camera.maxRadius = 800.0f;
    camera.orbitRotateSpeed = 60.0f;
    camera.orbitZoomSpeed = 400.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f);
    camera.enableAutoOrbit = false;

    updateCamera();
}

void updateCamera(){
    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    float cosPitch = cos(pitchRad);

    camera.position.x = camera.target.x + camera.radius * cosPitch * cos(yawRad);
    camera.position.y = camera.target.y + camera.radius * sin(pitchRad);
    camera.position.z = camera.target.z + camera.radius * cosPitch * sin(yawRad);

    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta) {
    camera.yaw += yawDelta;
    camera.pitch = glm::clamp(camera.pitch + pitchDelta, camera.minOrbitPitch, camera.maxOrbitPitch);
    camera.radius = glm::clamp(camera.radius + radiusDelta, camera.minRadius, camera.maxRadius);
    updateCamera();
}

void light_setup(){
    light.position = glm::vec3(3.0f, 5.0f, 3.0f);
    light.color = glm::vec3(1.0);
}

void shader_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> shadingMethod = {
        "basic", 
        "bubble",
        "water", 
        "fur"
    };

    for(int i=0; i<shadingMethod.size(); i++){
        std::string vpath = shaderDir + shadingMethod[i] + ".vert";
        std::string fpath = shaderDir + shadingMethod[i] + ".frag";
        std::string gpath = shaderDir + shadingMethod[i] + ".geom";

        shader_program_t* shaderProgram = new shader_program_t();
        shaderProgram->create();
        if (i != 3)
        {
            shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
            shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        }
        else //fur
        {
            shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
            shaderProgram->add_shader(gpath, GL_GEOMETRY_SHADER);
            shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        }
        shaderProgram->link_shader();
        shaderPrograms.push_back(shaderProgram);
    }
}

void cubemap_setup(){
    std::string cubemapDir = "..\\..\\src\\asset\\background\\bathroom\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";

    std::vector<std::string> faces
    {
        cubemapDir + "right.jpg",
        cubemapDir + "left.jpg",
        cubemapDir + "top.jpg",
        cubemapDir + "bottom.jpg",
        cubemapDir + "front.jpg",
        cubemapDir + "back.jpg"
    };
    cubemapTexture = loadCubemap(faces);   

    std::string vpath = shaderDir + "cubemap.vert";
    std::string fpath = shaderDir + "cubemap.frag";
    
    cubemapShader = new shader_program_t();
    cubemapShader->create();
    cubemapShader->add_shader(vpath, GL_VERTEX_SHADER);
    cubemapShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    cubemapShader->link_shader();

    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glBindVertexArray(cubemapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void setup(){
    light_setup();
    model_setup();
    shader_setup();
    camera_setup();
    cubemap_setup();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void update(){
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;

    if (SoapShow)
    {
        if (SoapShow && currentTime - SoapStartTime > 1.5f) 
        {
            BubbleAmount += BubbleGrowSpeed * deltaTime;
            BubbleShow = true;
        }
        if (currentTime - SoapStartTime > SoapShowTime) SoapShow = false;
    }

    if (WaterShow)
    {
        if (currentTime - WaterStartTime > WaterShowTime) WaterShow = false;
    }

    if (DryerShow && currentTime - DryerStartTime >= 0.5f)
    {
        FurStrength += FurGrowSpeed * deltaTime;
        if (currentTime - DryerStartTime > DryerShowTime) DryerShow = false;
    }
    else FurStrength -= FurSmallSpeed * deltaTime;

    FurStrength = glm::clamp(FurStrength, minStrength, maxStrength);
}

void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 modelMatrix(1.0f);
    
    if (DogJump)
    {
        float t = currentTime - DogJumpStartTime;
        t = glm::clamp(t / DogJumpDuration, 0.0f, 1.0f);

        //跳躍高度
        float H = sin(t * glm::pi<float>()) * 80.0f;

        //旋轉2圈
        float Angle = t * 720.0f;

        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, H, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(Angle), glm::vec3(0.0f, 1.0f, 0.0f));

        if (t == 1.0f) DogJump = false;
    }
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(100.0f));

    glm::mat4 view = glm::lookAt(camera.position, camera.target, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    //basic
    shaderPrograms[0]->use();
    shaderPrograms[0]->set_uniform_value("model", modelMatrix);
    shaderPrograms[0]->set_uniform_value("view", view);
    shaderPrograms[0]->set_uniform_value("projection", projection);
    shaderPrograms[0]->set_uniform_value("viewPos", camera.position);
    shaderPrograms[0]->set_uniform_value("light.position", light.position);
    shaderPrograms[0]->set_uniform_value("light.color", light.color);
    shaderPrograms[0]->set_uniform_value("ourTexture", 0);
    dogModel->draw();
    shaderPrograms[0]->release();

    if (SoapShow)
    {
        glm::mat4 soapModelMatrix = glm::mat4(1.0f);

        float t = currentTime - SoapStartTime;
        float offsetX = sin(t * 6.0f) * 10.0f;
        float offsetY = sin(t * 6.0f) * 5.0f;

        soapModelMatrix = glm::translate(soapModelMatrix, glm::vec3(offsetX - 60.0f, offsetY - 25.0f, 0.0f));
        soapModelMatrix = glm::scale(soapModelMatrix, glm::vec3(0.75f));
        soapModelMatrix = glm::rotate(soapModelMatrix, glm::radians(17.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        shaderPrograms[0]->use();
        shaderPrograms[0]->set_uniform_value("model", soapModelMatrix);
        shaderPrograms[0]->set_uniform_value("view", view);
        shaderPrograms[0]->set_uniform_value("projection", projection);
        shaderPrograms[0]->set_uniform_value("ourTexture", 0);

        soapModel->draw();
        shaderPrograms[0]->release();
    }

    if (BubbleShow)
    {
        glEnable(GL_BLEND);
        shaderPrograms[1]->use();
        shaderPrograms[1]->set_uniform_value("model", modelMatrix);
        shaderPrograms[1]->set_uniform_value("view", view);
        shaderPrograms[1]->set_uniform_value("projection", projection);
        shaderPrograms[1]->set_uniform_value("viewPos", camera.position);
        shaderPrograms[1]->set_uniform_value("ourTexture", 0);

        shaderPrograms[1]->set_uniform_value("BubbleAmount", BubbleAmount);

        dogModel->draw();
        shaderPrograms[1]->release();
    }

    if (WaterShow)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shaderPrograms[2]->use();
        shaderPrograms[2]->set_uniform_value("view", view);
        shaderPrograms[2]->set_uniform_value("projection", projection);
        shaderPrograms[2]->set_uniform_value("viewPos", camera.position);
        shaderPrograms[2]->set_uniform_value("colorFilter", glm::vec3(0.0f, 0.5f, 1.0f)); 
        
        float t = currentTime - WaterStartTime;
        float alphaValue = 1.0f;

        float growRate = 32.0f;
        int maxBubbles = 32;
        float currentCount = t * growRate; 
        int numToDraw = std::min((int)ceil(currentCount), maxBubbles);

        float radius;
        float posY;
        float startY = 40.0f;
        float targetY = -60.0f;
        float fallSpeed = 40.0f;
        float tStopStart = (startY - targetY) / fallSpeed;
        float stopDuration = 4.0f;
        float tStopEnd = tStopStart + stopDuration;

        if (t < tStopStart) {
            posY = startY - fallSpeed * t;
            radius = 120.0f;
        } else if (t < tStopEnd) {
            posY = targetY;
            radius = 120.0f;
        } else {
            float tAfterStop = t - tStopEnd;
            posY = targetY - fallSpeed * tAfterStop;
            radius = 120.0f - tAfterStop * 20.0f;
            if (radius < 0.0f) radius = 0.0f;

            alphaValue = glm::clamp(1.0f - tAfterStop / 2.0f, 0.0f, 1.0f);
        }

        shaderPrograms[2]->set_uniform_value("alpha", alphaValue);

        for (int i = 0; i < numToDraw; i++)
        {
            float angle = glm::radians(i * (360.0f / maxBubbles) + t * 50.0f); 
            float posX = cos(angle) * radius;
            float posZ = sin(angle) * radius;
            posY += sin(t * 2.0f + i) * 10.0f;

            float maturity = glm::clamp(currentCount - (float)i, 0.0f, 1.0f);

            glm::mat4 waterModelMatrix = glm::mat4(1.0f);
            waterModelMatrix = glm::translate(waterModelMatrix, glm::vec3(posX - 30.0f, posY, posZ));
            
            float pulse = (0.9f + sin(t * 3.0f + i) * 0.2f) * maturity;
            waterModelMatrix = glm::scale(waterModelMatrix, glm::vec3(pulse));

            shaderPrograms[2]->set_uniform_value("model", waterModelMatrix);
            waterModel->draw();
        }
        if (t > 5.0f)
        {
            BubbleShow = false;
            glDisable(GL_BLEND);
        }

        shaderPrograms[2]->release();
    }

    if (DryerShow)
    {
        glm::mat4 dryerModelMatrix = glm::mat4(1.0f);

        float t = currentTime - DryerStartTime;

        if (t <= 2.5f)
        {
            float offsetX = sin(t * 6.0f) * 10.0f;
            float offsetZ = sin(t * 6.0f) * 5.0f;
            float rotationY = sin(t * 6.0f) * 2.5f;

            dryerModelMatrix = glm::mat4(1.0f);
            dryerModelMatrix = glm::translate(dryerModelMatrix, glm::vec3(offsetX + 180.0f, 150.0f, offsetZ - 120.0f));
            dryerModelMatrix = glm::scale(dryerModelMatrix, glm::vec3(20.0f));
            dryerModelMatrix = glm::rotate(dryerModelMatrix, glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            dryerModelMatrix = glm::rotate(dryerModelMatrix, glm::radians(30.0f + rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            
            shaderPrograms[0]->use();
            shaderPrograms[0]->set_uniform_value("model", dryerModelMatrix);
            shaderPrograms[0]->set_uniform_value("view", view);
            shaderPrograms[0]->set_uniform_value("projection", projection);

            dryerModel->draw();
            shaderPrograms[0]->release();
        }
        else
        {
            float offsetX = sin(t * 6.0f) * 10.0f;
            float offsetZ = sin(t * 6.0f) * 5.0f;
            float rotationY = sin(t * 6.0f) * 2.5f;

            dryerModelMatrix = glm::mat4(1.0f);
            dryerModelMatrix = glm::translate(dryerModelMatrix, glm::vec3(offsetX - 20.0f, 210.0f, offsetZ - 150.0f));
            dryerModelMatrix = glm::scale(dryerModelMatrix, glm::vec3(20.0f));
            dryerModelMatrix = glm::rotate(dryerModelMatrix, glm::radians(90.0f + rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            dryerModelMatrix = glm::rotate(dryerModelMatrix, glm::radians(60.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            
            shaderPrograms[0]->use();
            shaderPrograms[0]->set_uniform_value("model", dryerModelMatrix);
            shaderPrograms[0]->set_uniform_value("view", view);
            shaderPrograms[0]->set_uniform_value("projection", projection);

            dryerModel->draw();
            shaderPrograms[0]->release();
        }
    }

    if (shaderProgramIndex == 3)
    {
        float t = currentTime - DryerStartTime;
        shaderPrograms[3]->use();
        shaderPrograms[3]->set_uniform_value("model", modelMatrix);
        shaderPrograms[3]->set_uniform_value("view", view);
        shaderPrograms[3]->set_uniform_value("projection", projection);
        shaderPrograms[3]->set_uniform_value("viewPos", camera.position);
        shaderPrograms[3]->set_uniform_value("ourTexture", 0);

        shaderPrograms[3]->set_uniform_value("Strength", FurStrength);
        shaderPrograms[3]->set_uniform_value("Length", 0.8f);
        
        dogModel->draw();
        shaderPrograms[3]->release();
    }

    cubemapShader->use();
    cubemapShader->set_uniform_value("view", view);
    cubemapShader->set_uniform_value("projection", projection);
   
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    cubemapShader->set_uniform_value("skybox", 0);

    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ICG Final Project", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    setup();
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        update();
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete dogModel;
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    glm::vec2 orbitInput(0.0f);
    float zoomInput = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        orbitInput.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        orbitInput.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        orbitInput.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        orbitInput.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        zoomInput -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        zoomInput += 1.0f;

    if (orbitInput.x != 0.0f || orbitInput.y != 0.0f || zoomInput != 0.0f) {
        float yawDelta = orbitInput.x * camera.orbitRotateSpeed * deltaTime;
        float pitchDelta = orbitInput.y * camera.orbitRotateSpeed * deltaTime;
        float radiusDelta = zoomInput * camera.orbitZoomSpeed * deltaTime;
        applyOrbitDelta(yawDelta, pitchDelta, radiusDelta);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) 
    {
        shaderProgramIndex = 0; //basic

        SoapShow = false;
        BubbleAmount = 0.0f;

        BubbleShow = false;
        WaterShow = false;
        DryerShow = false;
        DogJump = false;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) 
    {
        shaderProgramIndex = 1; //bubble
        SoapShow = true;
        SoapStartTime = glfwGetTime();
        BubbleShow = false;
        BubbleAmount = 0.0f;

        WaterShow = false;
        DryerShow = false;
        DogJump = false;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        shaderProgramIndex = 2; //water
        WaterShow = true;
        WaterStartTime = glfwGetTime();

        SoapShow = false;
        DryerShow = false;
        DogJump = false;
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        shaderProgramIndex = 3; //fur
        DryerShow = true;
        DryerStartTime = glfwGetTime();

        SoapShow = false;
        BubbleAmount = 0.0f;

        WaterShow = false;
        DogJump = false;
    }
    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        DogJump = true;
        DogJumpStartTime = glfwGetTime();

        SoapShow = false;
        BubbleShow = false;
        WaterShow = false;
        DryerShow = false;
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

unsigned int loadCubemap(vector<std::string>& faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}  
