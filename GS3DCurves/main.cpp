#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <random>

#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int modsdouble);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void RenderUI();
glm::vec3 camPos = glm::vec3(10.0f, 10.0f, 10.0f);
glm::vec3 camFront = glm::vec3(-1.0f, -1.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 0.0f, 1.0f);
float sensitivity = 5.0f;
bool focused = false;

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 1920.0f / 2.0;
float lastY = 1080.0 / 2.0;
float fov = 45.0f;
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

struct Node {
    int config;
    glm::vec3 pos;
    glm::vec3 e1[2];
    glm::vec3 e2[2];
    glm::vec3 e3[2];
};

struct Curve {


};

int main() {
    //set glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Curves for GS", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW Window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to init GLAD");
        return -1;
    }

    int row = 5;
    int col = 5;
    int dep = 5;
    int space = 2;
    int len = row * col * dep;
    Node*** allNodes;
    allNodes = new Node * *[row];

    // construct nodes
    // all of them spaced apart by 'space'
    // 3 end points, e1(x direction), e2(y direction), e3(z direction)
    // end points are located at the space/4 distance from the center
    // end points within cells are distanced as space/2
    // end points between cells are distanced as space/2
    // do not have offset for now
    for (int i = 0; i < row; i++) {
        allNodes[i] = new Node * [col];
        for (int j = 0; j < col; j++) {
            allNodes[i][j] = new Node[dep];
            for (int k = 0; k < dep; k++) {
                allNodes[i][j][k].config = std::rand() % 8;
                allNodes[i][j][k].pos = glm::vec3(i*space,j*space,k*space);
                allNodes[i][j][k].e1[0] = glm::vec3(i * space - space/4, j * space, k * space);
                allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space);
                allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space);
                allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space);
                allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4);
                allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4);
            }
        }
    }

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            for (int k = 0; k < dep; k++) {
                
            }
        }
    }

    int width, height;
    Shader shader = Shader("","");
    /*
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    */
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        shader.setMat4("view", view);

        glfwGetWindowSize(window, &width, &height);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 100000.0f);
        
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        
        //start of imgui init stuff
        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();

        //ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //stbi_image_free(data);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (focused) {

        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = ypos - lastY; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float mouseSens = 0.2f;
        xoffset *= mouseSens;
        yoffset *= mouseSens;

        yaw -= xoffset;
        pitch -= yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.z = sin(glm::radians(pitch));
        camFront = glm::normalize(front);

    }
    else {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
        lastX = xpos;
        lastY = ypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int modsdouble)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (focused)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        focused = !focused;
    }
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

    sensitivity += 0.2f * static_cast<float>(yoffset);
    if (sensitivity < 0) {
        sensitivity = 0.01f;
    }

    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}
