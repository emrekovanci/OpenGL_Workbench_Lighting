#include <iostream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "headers/Vertex.hpp"
#include "headers/Shader.hpp"
#include "headers/FreeLookCamera.hpp"
#include "headers/TextureManager.hpp"
#include "headers/PointLight.hpp"
#include "headers/Mesh.hpp"
#include "headers/Model.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH * 0.5f;
float lastY = SCR_HEIGHT * 0.5f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader litShader("resources/shaders/vert_lit.glsl", "resources/shaders/frag_lit.glsl");
    Shader unlitShader("resources/shaders/vert_unlit.glsl", "resources/shaders/frag_unlit.glsl");
    TextureManager textureManager;
    unsigned int diffuseTexture = textureManager.load("resources/textures/container2.png");
    unsigned int specularTexture = textureManager.load("resources/textures/container2_specular.png");
    unsigned int emissionTexture = textureManager.load("resources/textures/matrix.jpg");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<Vertex> vertices {
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

    // first, configure the cube's VAO (and VBO)
    // -----------------------------------------
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // position attribute
    // ------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    // ----------------
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    // texcoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);

    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Setup Dear ImGui
    // ----------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    //Model backpackModel("models/backpack.obj");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        const float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrame;
        lastFrame = currentFrameTime;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view/projection transformations
        // -------------------------------
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // lit shader props
        // ----------------
        litShader.use();
        litShader.setFloat("time", currentFrameTime);
        litShader.setVec3("viewPos", camera.Position);
        litShader.setMat4("projection", projection);
        litShader.setMat4("view", view);

        /*// render model
        // -------------------
        {
            glm::mat4 trsBackpackModel =
                glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

            litShader.setMat4("model", trsBackpackModel);
            backpackModel.render(litShader);
        }*/

        // material props
        // --------------
        litShader.setInt("material.diffuse", 0);
        litShader.setInt("material.specular", 1);
        litShader.setInt("material.emission", 2);
        litShader.setFloat("material.shininess", 64.0f);

        // directional light
        // -----------------
        litShader.setVec3("directionalLight.direction", -0.2f, -1.0f, -0.3f);
        litShader.setVec3("directionalLight.ambient", 0.0f, 0.0f, 0.0f);
        litShader.setVec3("directionalLight.diffuse", 0.05f, 0.05f, 0.05f);
        litShader.setVec3("directionalLight.specular", 0.2f, 0.2f, 0.2f);

        // point lights
        // ------------
        for (std::size_t i = 0; i < pointLights.size(); ++i)
        {
            std::string number = std::to_string(i);
            std::string prefix = "pointLights[" + number + "]";

            litShader.setVec3(prefix + ".position", pointLights[i].position);
            litShader.setVec3(prefix + ".ambient", pointLights[i].color * 0.1f);
            litShader.setVec3(prefix + ".diffuse", pointLights[i].color);
            litShader.setVec3(prefix + ".specular", pointLights[i].color);
            litShader.setFloat(prefix + ".constant", pointLights[i].constant);
            litShader.setFloat(prefix + ".linear", pointLights[i].linear);
            litShader.setFloat(prefix + ".quadratic", pointLights[i].quadratic);
        }

        // spotLight
        // ---------
        litShader.setVec3("spotLight.position", camera.Position);
        litShader.setVec3("spotLight.direction", camera.Front);
        litShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        litShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        litShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        litShader.setFloat("spotLight.constant", 1.0f);
        litShader.setFloat("spotLight.linear", 0.09f);
        litShader.setFloat("spotLight.quadratic", 0.032f);
        litShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
        litShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // render cubes
        // ------------
        textureManager.activate(GL_TEXTURE0, diffuseTexture);
        textureManager.activate(GL_TEXTURE1, specularTexture);
        textureManager.activate(GL_TEXTURE2, emissionTexture);
        glBindVertexArray(cubeVAO);
        for (std::size_t i = 0; i < cubePositions.size(); ++i)
        {
            const float angle = currentFrameTime * (20.0f * i);

            glm::mat4 trsLitCube =
                glm::translate(glm::mat4(1.0f), cubePositions[i]) *
                glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));

            litShader.setMat4("model", trsLitCube);

            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        unlitShader.use();
        unlitShader.setMat4("projection", projection);
        unlitShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);
        for (std::size_t i = 0; i < pointLights.size(); ++i)
        {
            glm::mat4 trsUnlitCube =
                glm::translate(glm::mat4(1.0f), pointLights[i].position) *
                glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

            unlitShader.setMat4("model", trsUnlitCube);

            glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        }

        // imgui
        // -----
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

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.ProcessKeyboard(FORWARD, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.ProcessKeyboard(BACKWARD, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.ProcessKeyboard(LEFT, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.ProcessKeyboard(RIGHT, deltaTime); }
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}