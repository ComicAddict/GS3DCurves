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
float yaw = 90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 1920.0f / 2.0;
float lastY = 1080.0 / 2.0;
float fov = 45.0f;
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float deltaTimeFrame = .0f;
float lastFrame = .0f;

/*
* TODO:
    shadow
    camera priority
    connections

    cubic interpolation
    b-spline ya da

    rule based - generation
*/

struct Node {
    int config;
    glm::vec3 pos;
    glm::vec3 e1[2];
    glm::vec3 e2[2];
    glm::vec3 e3[2];
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

void deleteStructure(Node***& nodes, int dims[3]) {
    /*
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            delete[] nodes[i][j];
        }
        delete nodes[i];
    }*/
    delete[] nodes;
}

void initStructure(Node***& nodes, int dims[3]) {
    nodes = new Node ** [dims[0]];
    for (int i = 0; i < dims[0]; i++) {
        nodes[i] = new Node * [dims[1]];
        for (int j = 0; j < dims[1]; j++) {
            nodes[i][j] = new Node[dims[2]];
        }
    }
}

void randomizeStructure(Node***& nodes, int dims[3], int config) {
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                nodes[i][j][k].config = (std::rand() % 8) & config;
            }
        }
    }
}

void abcStructure(Node***& nodes, int dims[3], int config, int a, int b, int c) {
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if((i+c*(j+k)) % (a+b) < a)
                    nodes[i][j][k].config = 0;
                else
                    nodes[i][j][k].config = config;
            }
        }
    }
}

void randomizeStructure(Node***& nodes, int dims[3]) {
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                nodes[i][j][k].config = std::rand() % 8;
            }
        }
    }
}

void updateStructure(Node***& nodes, int dims[3], float space) {
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                //printf("config of (%i, %i, %i) is %i\n", i, j, k, config);
                nodes[i][j][k].pos = glm::vec3(i * space, j * space, k * space);
                nodes[i][j][k].e1[0] = space * getDispNodeX(0) / 4.0f;
                nodes[i][j][k].e1[1] = space * getDispNodeX(0) / 4.0f;
                nodes[i][j][k].e2[0] = space * getDispNodeY(2) / 4.0f;
                nodes[i][j][k].e2[1] = space * getDispNodeY(2) / 4.0f;
                nodes[i][j][k].e3[0] = space * getDispNodeZ(3) / 4.0f;
                nodes[i][j][k].e3[1] = space * getDispNodeZ(3) / 4.0f;

                if (nodes[i][j][k].config & (1 << 0)) {
                    //printf("mirror about xy\n");
                    nodes[i][j][k].e1[0].z *= -1.0f;
                    nodes[i][j][k].e1[1].z *= -1.0f;
                    nodes[i][j][k].e2[0].z *= -1.0f;
                    nodes[i][j][k].e2[1].z *= -1.0f;
                    nodes[i][j][k].e3[0].z *= -1.0f;
                    nodes[i][j][k].e3[1].z *= -1.0f;
                }

                if (nodes[i][j][k].config & (1 << 1)) {
                    //printf("mirror about yz\n");
                    nodes[i][j][k].e1[0].x *= -1.0f;
                    nodes[i][j][k].e1[1].x *= -1.0f;
                    nodes[i][j][k].e2[0].x *= -1.0f;
                    nodes[i][j][k].e2[1].x *= -1.0f;
                    nodes[i][j][k].e3[0].x *= -1.0f;
                    nodes[i][j][k].e3[1].x *= -1.0f;
                }

                if (nodes[i][j][k].config & (1 << 2)) {
                    //printf("mirror about xz\n");
                    nodes[i][j][k].e1[0].y *= -1.0f;
                    nodes[i][j][k].e1[1].y *= -1.0f;
                    nodes[i][j][k].e2[0].y *= -1.0f;
                    nodes[i][j][k].e2[1].y *= -1.0f;
                    nodes[i][j][k].e3[0].y *= -1.0f;
                    nodes[i][j][k].e3[1].y *= -1.0f;
                }

                nodes[i][j][k].e1[0] += glm::vec3(i * space - space / 4, j * space, k * space);
                nodes[i][j][k].e1[1] += glm::vec3(i * space + space / 4, j * space, k * space);
                nodes[i][j][k].e2[0] += glm::vec3(i * space, j * space - space / 4, k * space);
                nodes[i][j][k].e2[1] += glm::vec3(i * space, j * space + space / 4, k * space);
                nodes[i][j][k].e3[0] += glm::vec3(i * space, j * space, k * space - space / 4);
                nodes[i][j][k].e3[1] += glm::vec3(i * space, j * space, k * space + space / 4);
            }
        }
    }
}

void generateRandomStructure(Node***& nodes, int dims[3], float space) {
    if (nodes != nullptr) {
        deleteStructure(nodes, dims);
    }
    initStructure(nodes, dims);
    randomizeStructure(nodes, dims);
    updateStructure(nodes, dims, space);
}

void generateRandomStructure(Node***& nodes, int dims[3], float space, int config) {
    if (nodes != nullptr) {
        deleteStructure(nodes, dims);
    }
    initStructure(nodes, dims);
    randomizeStructure(nodes, dims, config);
    updateStructure(nodes, dims, space);
}

void generateABCStructure(Node***& nodes, int dims[3], float space, int config, int a, int b, int c) {
    if (nodes != nullptr) {
        deleteStructure(nodes, dims);
    }
    initStructure(nodes, dims);
    abcStructure(nodes, dims, config, a, b, c);
    updateStructure(nodes, dims, space);
}

void generateStructureData(Node***& nodes, int dims[3], std::vector<glm::vec3>& v, std::vector<glm::vec3>& c) {
    v.clear();
    c.clear();
    std::vector<glm::vec3> edgesx;
    std::vector<glm::vec3> edgesy;
    std::vector<glm::vec3> edgesz;
    std::vector<glm::vec3> colx;
    std::vector<glm::vec3> coly;
    std::vector<glm::vec3> colz;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if (i != 0) {
                    edgesx.push_back(nodes[i - 1][j][k].e1[1]);
                    edgesx.push_back(nodes[i][j][k].e1[0]);
                    colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                    colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                }
                if (j != 0) {
                    edgesy.push_back(nodes[i][j - 1][k].e2[1]);
                    edgesy.push_back(nodes[i][j][k].e2[0]);
                    coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                    coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                }
                if (k != 0) {
                    edgesz.push_back(nodes[i][j][k - 1].e3[1]);
                    edgesz.push_back(nodes[i][j][k].e3[0]);
                    colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                    colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                }
                edgesx.push_back(nodes[i][j][k].e1[0]);
                edgesx.push_back(nodes[i][j][k].e1[1]);
                edgesy.push_back(nodes[i][j][k].e2[0]);
                edgesy.push_back(nodes[i][j][k].e2[1]);
                edgesz.push_back(nodes[i][j][k].e3[0]);
                edgesz.push_back(nodes[i][j][k].e3[1]);
                colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }
    }
    v.insert(v.end(), edgesx.begin(), edgesx.end());
    v.insert(v.end(), edgesy.begin(), edgesy.end());
    v.insert(v.end(), edgesz.begin(), edgesz.end());
    c.insert(c.end(), colx.begin(), colx.end());
    c.insert(c.end(), coly.begin(), coly.end());
    c.insert(c.end(), colz.begin(), colz.end());
}

void generateStructureData(Node***& nodes, int dims[3], std::vector<glm::vec3>& v, std::vector<glm::vec3>& c, int axis) {
    v.clear();
    c.clear();
    std::vector<glm::vec3> edgesx;
    std::vector<glm::vec3> edgesy;
    std::vector<glm::vec3> edgesz;
    std::vector<glm::vec3> colx;
    std::vector<glm::vec3> coly;
    std::vector<glm::vec3> colz;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if (i != 0) {
                    edgesx.push_back(nodes[i - 1][j][k].e1[1]);
                    edgesx.push_back(nodes[i][j][k].e1[0]);
                    colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                    colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                }
                if (j != 0) {
                    edgesy.push_back(nodes[i][j - 1][k].e2[1]);
                    edgesy.push_back(nodes[i][j][k].e2[0]);
                    coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                    coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                }
                if (k != 0) {
                    edgesz.push_back(nodes[i][j][k - 1].e3[1]);
                    edgesz.push_back(nodes[i][j][k].e3[0]);
                    colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                    colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                }
                edgesx.push_back(nodes[i][j][k].e1[0]);
                edgesx.push_back(nodes[i][j][k].e1[1]);
                edgesy.push_back(nodes[i][j][k].e2[0]);
                edgesy.push_back(nodes[i][j][k].e2[1]);
                edgesz.push_back(nodes[i][j][k].e3[0]);
                edgesz.push_back(nodes[i][j][k].e3[1]);
                colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                colx.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                coly.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                colz.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }
    }
    
    if (axis & (1 << 0)) {
        v.insert(v.end(), edgesx.begin(), edgesx.end());
        c.insert(c.end(), colx.begin(), colx.end());
    }
    
    if (axis & (1 << 1)) {
        v.insert(v.end(), edgesy.begin(), edgesy.end());
        c.insert(c.end(), coly.begin(), coly.end());
    }
    
    if (axis & (1 << 2)) {
        v.insert(v.end(), edgesz.begin(), edgesz.end());
        c.insert(c.end(), colz.begin(), colz.end());
    }
    
    
    
    
}

void updateBufferData(unsigned int& bufIndex, std::vector<glm::vec3>& data) {
    glBindBuffer(GL_ARRAY_BUFFER, bufIndex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * data.size(), &data[0], GL_DYNAMIC_DRAW);
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

    int dims[3] = { 1,1,1 };
    float space = 4.0f;
    int len = dims[0] * dims[1] * dims[2];
    Node*** allNodes = nullptr;
    generateRandomStructure(allNodes, dims, space);
   
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> c;
    generateStructureData(allNodes, dims, v, c);
    unsigned int vao, vbo_pos, vbo_col;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_col);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * v.size(), &v[0], GL_DYNAMIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * c.size(), &c[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    int width, height;
    Shader shader = Shader("C:\\Src\\shaders\\linevertex.glsl", "C:\\Src\\shaders\\linefrag.glsl", "C:\\Src\\shaders\\cylinder.glsl");
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    float radius = 0.3f;
    float orthoScale = 10.0f;
    glm::vec3 lightPos = { 10.0f,10.f,10.f };
    bool xz = true, yz = true, xy = true, xyz = false, x = true, y = true, z = true;
    int config = 7;
    int axis = 7;
    int ABC[3] = { 1,1,1 };

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    //ImGui::SetNextWindowPos(viewport->Pos);
    //ImGui::SetNextWindowSize(viewport->Size);
    //ImGui::SetNextWindowViewport(viewport->ID);
    bool ortho = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTimeFrame = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        // render
        // ------
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        shader.setMat4("view", view);

        glfwGetWindowSize(window, &width, &height);

        float ratio = (float)width / (float)height;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.01f, 100000.0f);
        if(ortho)
            projection = glm::ortho(-orthoScale * ratio, orthoScale * ratio, -orthoScale, orthoScale, -1000.0f, 1000.0f);
        
        shader.setMat4("projection", projection);
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        shader.setFloat("rad", radius);
        shader.setVec3("lightPos", lightPos);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, v.size());

        //start of imgui init stuff
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
        ImGui::Begin("Structure Settings");
        if (ImGui::DragInt3("Dimension", dims, .2f, 2, 100)) {
            if(!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::DragFloat("Space", &space, 0.1f, 0.01f, 100.0f)) {
            updateStructure(allNodes, dims, space);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Enable XY Mirror", &xy)) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Enable XZ Mirror", &xz) ) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Enable YZ Mirror", &yz)) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Only XYZ Mirror", &xyz)) {
            xy = true;
            yz = true;
            xz = true;
            config = xy * 1 + yz * 2 + xz * 4;
            generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::DragInt3("ABC", ABC, .2f, 1, 70)) {
            generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("X Strands", &x)) {
            axis = x * 1 + y * 2 + z * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Y Strands", &y)) {
            axis = x * 1 + y * 2 + z * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        if (ImGui::Checkbox("Z Strands", &z)) {
            axis = x * 1 + y * 2 + z * 4;
            if (!xyz)
                generateRandomStructure(allNodes, dims, space, config);
            else
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureData(allNodes, dims, v, c, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
        }
        ImGui::DragFloat("Radius", &radius, 0.01);
        ImGui::InputFloat("Resolution", &radius);
        ImGui::End();

        ImGui::Begin("Render Settings");
        ImGui::DragFloat3("Light Pos", &lightPos[0], 0.05);
        ImGui::Checkbox("Orthographic", &ortho);
        if(ortho)
            ImGui::DragFloat("Orthographic Scale", &orthoScale, 0.05, 0.01f, 100.0f);
        ImGui::End();
        ImGui::Render();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //stbi_image_free(data);
    deleteStructure(allNodes, dims);
    glfwTerminate();
    return 0;
}

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

