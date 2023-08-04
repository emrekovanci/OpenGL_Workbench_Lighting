#include <iostream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Vertex.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "TextureManager.hpp"
#include "PointLight.hpp"
#include "Mesh.hpp"
#include "Model.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera;
float lastX = SCR_WIDTH * 0.5f;
float lastY = SCR_HEIGHT * 0.5f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

TextureManager textureManager;

void renderCubes(Shader& shader, unsigned int vao, const std::vector<glm::vec3>& positions, const glm::mat4& projection, const glm::mat4& view, float currentFrameTime)
{
    shader.use();
    shader.setFloat("time", currentFrameTime);
    shader.setVec3("viewPos", camera.Position);
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    textureManager.activate(GL_TEXTURE0, textureManager.get("diffuse"));
    textureManager.activate(GL_TEXTURE1, textureManager.get("specular"));
    textureManager.activate(GL_TEXTURE2, textureManager.get("emission"));

    glBindVertexArray(vao);
    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        glm::mat4 trs =
            glm::translate(glm::mat4(1.0f), positions[i]) *
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

        shader.setMat4("model", trs);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void updatePointLights(Shader& shader, std::vector<PointLight>& pointLights)
{
    shader.use();

    for (std::size_t i = 0; i < pointLights.size(); ++i)
    {
        std::string number = std::to_string(i);
        std::string prefix = "pointLights[" + number + "]";

        shader.setVec3(prefix + ".position", pointLights[i].position);
        shader.setVec3(prefix + ".ambient", pointLights[i].color * 0.1f);
        shader.setVec3(prefix + ".diffuse", pointLights[i].color);
        shader.setVec3(prefix + ".specular", pointLights[i].color);
        shader.setFloat(prefix + ".constant", pointLights[i].constant);
        shader.setFloat(prefix + ".linear", pointLights[i].linear);
        shader.setFloat(prefix + ".quadratic", pointLights[i].quadratic);
    }
}

void updateSpotlight(Shader& shader)
{
    shader.use();
    shader.setVec3("spotLight.position", camera.Position);
    shader.setVec3("spotLight.direction", camera.Front);
    shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

void updateDirLight(Shader& shader)
{
    shader.use();
    shader.setVec3("directionalLight.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("directionalLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("directionalLight.diffuse", 0.05f, 0.05f, 0.05f);
    shader.setVec3("directionalLight.specular", 0.2f, 0.2f, 0.2f);
}

void renderPointLights(Shader& shader, unsigned int vao, const std::vector<PointLight>& lights, const glm::mat4& projection, const glm::mat4& view)
{
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glBindVertexArray(vao);
    for (std::size_t i = 0; i < lights.size(); ++i)
    {
        glm::mat4 trsUnlitCube =
            glm::translate(glm::mat4(1.0f), lights[i].position) *
            glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

        shader.setMat4("model", trsUnlitCube);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
}

void renderImGui(std::vector<PointLight>& pointLights)
{
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::Begin("Editor");

        for (std::size_t i = 0; i < pointLights.size(); ++i)
        {
            ImGui::PushID(i);
            std::string groupTitle = "Point Light: " + std::to_string(i);
            if (ImGui::CollapsingHeader(groupTitle.c_str()))
            {
                ImGui::SliderFloat3("Position", glm::value_ptr(pointLights[i].position), -50.0f, 50.0f);
                ImGui::SliderFloat3("Color", glm::value_ptr(pointLights[i].color), 0.0f, 1.0f);
                ImGui::SliderFloat("Constant", &pointLights[i].constant, 0.0f, 1.0f);
                ImGui::SliderFloat("Linear", &pointLights[i].linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Quadratic", &pointLights[i].quadratic, 0.0f, 1.0f);
            }
            ImGui::PopID();
        }

        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Chimpey!", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    Shader litShader("resources/shaders/vert_lit.glsl", "resources/shaders/frag_lit.glsl");
    litShader.use();
    litShader.setInt("material.diffuse", 0);
    litShader.setInt("material.specular", 1);
    litShader.setInt("material.emission", 2);
    litShader.setFloat("material.shininess", 64.0f);

    Shader unlitShader("resources/shaders/vert_unlit.glsl", "resources/shaders/frag_unlit.glsl");

    textureManager.load("resources/textures/container2.png", "diffuse");
    textureManager.load("resources/textures/container2_specular.png", "specular");
    textureManager.load("resources/textures/matrix.jpg", "emission");

    std::vector<Vertex> vertices
    {
        // back face
        { { -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },

        // front face
        { { -0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { 0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },

        // left face
        { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
        { { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
        { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },

        // right face
        { { 0.5f,  0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
        { { 0.5f,  0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f, -0.5f }, { 1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
        { { 0.5f, -0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
        { { 0.5f,  0.5f,  0.5f }, { 1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },

        // bottom face
        { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },

        // top face
        { { -0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { 0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } }
    };

    std::vector<glm::vec3> cubePositions {
        { 0.0f, -10.0f, 0.0f },
        { 2.0f, -5.0f, -15.0f },
        { -1.5f, -12.2f, -2.5f },
        { -3.8f, -12.0f, -12.3f },
        { 2.4f, -10.4f, -3.5f },
        { -1.7f, -7.0f, -7.5f },
        { 1.3f, -12.0f, -2.5f },
        { 1.5f, -8.0f, -2.5f },
        { 1.5f, -12.2f, -1.5f },
        { -1.3f, -11.0f, -1.5f }
    };

    std::vector<PointLight> pointLights {
        { { 0.7f, 0.2f, 2.0f }, { 0.1f, 0.1f, 0.1f } },
        { { 2.3f, -3.3f, -4.0f }, { 0.1f, 0.1f, 0.1f } },
        { { -4.0f, 2.0f, -12.0f }, { 0.1f, 0.1f, 0.1f } },
        { { 0.0f, 0.0f, -3.0f }, { 0.3f, 0.1f, 0.1f } }
    };

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup Dear ImGui
    // ----------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    //Model backpackModel("models/backpack.obj");

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        const float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrame;
        lastFrame = currentFrameTime;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // update light props
        updateDirLight(litShader);
        updatePointLights(litShader, pointLights);
        updateSpotlight(litShader);

        // render scene
        renderCubes(litShader, cubeVAO, cubePositions, projection, view, currentFrameTime);
        renderPointLights(unlitShader, lightCubeVAO, pointLights, projection, view);
        renderImGui(pointLights);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Forward, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Backward, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Left, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(CameraMovement::Right, deltaTime); }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}