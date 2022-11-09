#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "shader.h"
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

float deltaTimeFrame = .0f;
float lastFrame = .0f;

struct Node {
    int config;
    glm::vec3 pos;
    glm::vec3 e1[2];
    glm::vec3 e2[2];
    glm::vec3 e3[2];
};

struct Curve {


};

glm::vec3 getDispNodeX(int config) {
    switch (config)
    {
    case 0:
        return glm::vec3(0.0f,1.0f,1.0f);
    case 1:
        return glm::vec3(0.0f, -1.0f, 1.0f);
    case 2:
        return glm::vec3(0.0f, 1.0f, -1.0f);
    case 3:
        return glm::vec3(0.0f, -1.0f, -1.0f);
    default:
        return glm::vec3(-1.0f, -1.0f, 0.0f);
    }
}

glm::vec3 getDispNodeY(int config) {
    switch (config)
    {
    case 0:
        return glm::vec3(1.0f, 0.0f, 1.0f);
    case 1:
        return glm::vec3(-1.0f, 0.0f, 1.0f);
    case 2:
        return glm::vec3(1.0f, 0.0f, -1.0f);
    case 3:
        return glm::vec3(-1.0f, 0.0f, -1.0f);
    default:
        return glm::vec3(-1.0f, -1.0f, 0.0f);
    }
}

glm::vec3 getDispNodeZ(int config) {
    switch (config)
    {
    case 0:
        return glm::vec3(1.0f, 1.0f, 0.0f);
    case 1:
        return glm::vec3(-1.0f, 1.0f, 0.0f);
    case 2:
        return glm::vec3(1.0f, -1.0f, 0.0f);
    case 3:
        return glm::vec3(-1.0f, -1.0f, 0.0f);
    default:
        return glm::vec3(-1.0f, -1.0f, 0.0f);
    }
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camSpeed = static_cast<float>(sensitivity * deltaTimeFrame);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= camSpeed * glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += camSpeed * glm::normalize(glm::cross(camFront, camUp));
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camPos += camSpeed * camUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camPos -= camSpeed * camUp;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to init GLAD");
        return -1;
    }

    int row = 3;
    int col = 3;
    int dep = 3;
    float space = 4.0f;
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
                int config = std::rand() % 8;
                allNodes[i][j][k].config = config;
                //printf("config of (%i, %i, %i) is %i\n", i, j, k, config);
                allNodes[i][j][k].pos = glm::vec3(i * space, j * space, k * space);
                switch (config)
                {
                case 0:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(0) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(0) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(2) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(2) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(3) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(3) / 4.0f;
                    break;
                case 1:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(3) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(3) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(1) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(1) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(0) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(0) / 4.0f;
                    break;
                case 2:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(1) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(1) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(2) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(2) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(1) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(1) / 4.0f;
                    break;
                case 3:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(2) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(2) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(1) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(1) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(2) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(2) / 4.0f;
                    break;
                case 4:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(0) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(0) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(3) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(3) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(2) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(2) / 4.0f;
                    break;
                case 5:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(3) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(3) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(0) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(0) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(1) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(1) / 4.0f;
                    break;
                case 6:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(1) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(1) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(3) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(3) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(0) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(0) / 4.0f;
                    break;
                case 7:
                    allNodes[i][j][k].e1[0] = glm::vec3(i * space - space / 4, j * space, k * space) + space * getDispNodeX(2) / 4.0f;
                    allNodes[i][j][k].e1[1] = glm::vec3(i * space + space / 4, j * space, k * space) + space * getDispNodeX(2) / 4.0f;
                    allNodes[i][j][k].e2[0] = glm::vec3(i * space, j * space - space / 4, k * space) + space * getDispNodeY(0) / 4.0f;
                    allNodes[i][j][k].e2[1] = glm::vec3(i * space, j * space + space / 4, k * space) + space * getDispNodeY(0) / 4.0f;
                    allNodes[i][j][k].e3[0] = glm::vec3(i * space, j * space, k * space - space / 4) + space * getDispNodeZ(3) / 4.0f;
                    allNodes[i][j][k].e3[1] = glm::vec3(i * space, j * space, k * space + space / 4) + space * getDispNodeZ(3) / 4.0f;
                    break;
                
                }
                }
        }
    }
    std::vector<glm::vec3> edgesx;
    std::vector<glm::vec3> edgesy;
    std::vector<glm::vec3> edgesz;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            for (int k = 0; k < dep; k++) {
                if (i != 0) {
                    edgesx.push_back(allNodes[i-1][j][k].e1[1]);
                    edgesx.push_back(allNodes[i][j][k].e1[0]);
                }
                if (j != 0) {
                    edgesy.push_back(allNodes[i][j - 1][k].e2[1]);
                    edgesy.push_back(allNodes[i][j][k].e2[0]);
                }
                if (k != 0) {
                    edgesz.push_back(allNodes[i][j][k - 1].e3[1]);
                    edgesz.push_back(allNodes[i][j][k].e3[0]);
                }
                edgesx.push_back(allNodes[i][j][k].e1[0]);
                edgesx.push_back(allNodes[i][j][k].e1[1]);
                edgesy.push_back(allNodes[i][j][k].e2[0]);
                edgesy.push_back(allNodes[i][j][k].e2[1]);
                edgesz.push_back(allNodes[i][j][k].e3[0]);
                edgesz.push_back(allNodes[i][j][k].e3[1]);
            }
        }
    }
    edgesx.insert(edgesx.end(), edgesy.begin(), edgesy.end());
    edgesx.insert(edgesx.end(), edgesz.begin(), edgesz.end());

    //edgesx.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    //edgesx.push_back(glm::vec3(0.0f, 0.0f, 10.0f));

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * edgesx.size(), &edgesx[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int VAO_plane;
    glGenVertexArrays(1, &VAO_plane);

    float planeVertices[] = {
        10.0f,10.0f,0.0f,
        -10.0f,10.0f,0.0f,
        -10.0f,-10.0f,0.0f,
        -10.0f,-10.0f,0.0f,
        10.0f,-10.0f,0.0f,
        10.0f,10.0f,0.0f
    };

    unsigned int VBO_pos_p;
    glGenBuffers(1, &VBO_pos_p);

    glBindVertexArray(VAO_plane);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_p);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int width, height;
    Shader shader = Shader("C:\\Src\\shaders\\linevertex.glsl","C:\\Src\\shaders\\linefrag.glsl");
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);


    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTimeFrame = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        //printf("campos (%f,%f,%f)  ", camPos.x, camPos.y, camPos.z);
        //printf("camfront (%f,%f,%f)\n", camFront.x, camFront.y, camFront.z);
        // render
        // ------
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        shader.setMat4("view", view);

        glfwGetWindowSize(window, &width, &height);
        
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 100000.0f);
        shader.setMat4("projection", projection);
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, edgesx.size());

        //glBindVertexArray(VAO_plane);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
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
    delete allNodes;
    glfwTerminate();
    return 0;
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