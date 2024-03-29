#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <random>
#include <ostream>
#include <fstream>

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

const float PI = 3.14159265359f;
float deltaTimeFrame = .0f;
float lastFrame = .0f;

/*
* TODO:
    shadow
    camera priority
    + connections

    cubic interpolation
    b-spline ya da

    rule based - generation 
    + implemented ABC already

    + pipe interface between edges
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


void incStructure(Node***& nodes, int dims[3], int config) {
    bool xy = 1;// config& (1 << 0);
    bool yz = 1;//config & (1 << 1);
    bool xz = 1;//config & (1 << 2);
    printf("configs: %d, %d, %d\n", xy, yz, xz);
    nodes[0][0][0].config = config;
    for (int i = 0; i < dims[0]; i++) {
        if (i != 0)
            nodes[i][0][0].config = nodes[i - 1][0][0].config ^ (1 << 0);
        for (int j = 0; j < dims[1]; j++) {
            if (j != 0)
                nodes[i][j][0].config = nodes[i][j - 1][0].config ^ (1 << 1);
            for (int k = 0; k < dims[2]; k++) {
                if (k != 0)
                    nodes[i][j][k].config = nodes[i][j][k - 1].config ^ (1 << 2);
            }
        }
    }
}

void inc2Structure(Node***& nodes, int dims[3], int config) {
    bool xy = 1;// config& (1 << 0);
    bool yz = 1;//config & (1 << 1);
    bool xz = 1;//config & (1 << 2);
    printf("configs: %d, %d, %d\n", xy, yz, xz);
    nodes[0][0][0].config = config;
    for (int i = 0; i < dims[0]; i++) {
        if (i != 0)
            nodes[i][0][0].config = (nodes[i - 1][0][0].config + 1) % 8;
        for (int j = 0; j < dims[1]; j++) {
            if (j != 0)
                nodes[i][j][0].config = (nodes[i][j - 1][0].config + 1) % 8;
            for (int k = 0; k < dims[2]; k++) {
                if (k != 0)
                    nodes[i][j][k].config = (nodes[i][j][k - 1].config + 1) % 8;
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

void generateIncStructure(Node***& nodes, int dims[3], float space, int config) {
    if (nodes != nullptr) {
        deleteStructure(nodes, dims);
    }
    initStructure(nodes, dims);
    incStructure(nodes, dims, config);
    updateStructure(nodes, dims, space);
}


void generateInc2Structure(Node***& nodes, int dims[3], float space, int config) {
    if (nodes != nullptr) {
        deleteStructure(nodes, dims);
    }
    initStructure(nodes, dims);
    inc2Structure(nodes, dims, config);
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

glm::mat3x3 rotationMatrix(glm::vec3 axis, float angle)
{
    axis = glm::normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return glm::mat3x3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
    );
}

void generateStructureDataExp(Node***& nodes, int dims[3], std::vector<glm::vec3>& v, std::vector<glm::vec3>& c, std::vector<glm::vec3>& n, float radius, int res, float r, int res2, int axis) {
    v.clear();
    c.clear();
    n.clear();
    glm::vec3 v1;
    glm::vec3 v2;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if (axis & (1 << 0)) {
                    if (i != 0) {
                        int comp = ~(nodes[i - 1][j][k].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 0)) && (comp & (1 << 2))) {
                            v1 = nodes[i - 1][j][k].e1[1];
                            v2 = nodes[i][j][k].e1[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                        }
                        else {
                            v1 = nodes[i - 1][j][k].e1[1];
                            glm::vec3 dir = nodes[i][j][k].e1[0] - nodes[i - 1][j][k].e1[1];
                            glm::vec3 dir2 = dir;
                            dir2.x = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(1.0f, 0.0f, 0.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = PI / (2.0f * res2);
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(0.0f, 1.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(0.0f, cos(a2), sin(a2));
                            glm::vec3 d3(0.0f, 1.0f, 0.0f);
                            glm::vec3 d4(0.0f, cos(a2), sin(a2));
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.x/2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d2 - d1, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e1[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e1[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                            glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                            glm::vec3 norm(-1.0f, 0.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        }
                    }
                    if (i == (dims[0] - 1)) {
                        v1 = nodes[i][j][k].e1[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                            glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                            glm::vec3 norm(1.0f, 0.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e1[0];
                    v2 = nodes[i][j][k].e1[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                        glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                        glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                    }
                }
                if (axis & (1 << 1)) {
                    if (j != 0) {
                        int comp = ~(nodes[i][j - 1][k].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 0)) && (comp & (1 << 1))) {
                            v1 = nodes[i][j - 1][k].e2[1];
                            v2 = nodes[i][j][k].e2[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                        }
                        else {
                            v1 = nodes[i][j - 1][k].e2[1];
                            glm::vec3 dir = nodes[i][j][k].e2[0] - nodes[i][j - 1][k].e2[1];
                            glm::vec3 dir2 = dir;
                            dir2.y = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(0.0f, 1.0f, 0.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = PI / (2.0f * res2);
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(1.0f, 0.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(cos(a2), 0.0f, sin(a2));
                            glm::vec3 d3(1.0f, 0.0f, 0.0f);
                            glm::vec3 d4(cos(a2), 0.0f, sin(a2));
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.y / 2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d1 - d2, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d1 - d2, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d1 - d2, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e2[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e2[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                            glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                            glm::vec3 norm(0.0f, -1.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }
                    if (j == (dims[1] - 1)) {
                        v1 = nodes[i][j][k].e2[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                            glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                            glm::vec3 norm(0.0f, 1.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e2[0];
                    v2 = nodes[i][j][k].e2[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                        glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                        glm::vec3 normal = glm::normalize(glm::cross(v1 - v2, d2 - d1));
                        glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                }
                if (axis & (1 << 2)) {
                    if (k != 0) {
                        int comp = ~(nodes[i][j][k - 1].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 1)) && (comp & (1 << 2))) {
                            v1 = nodes[i][j][k - 1].e3[1];
                            v2 = nodes[i][j][k].e3[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                        }
                        else {
                            v1 = nodes[i][j][k - 1].e3[1];
                            glm::vec3 dir = nodes[i][j][k].e3[0] - nodes[i][j][k - 1].e3[1];
                            glm::vec3 dir2 = dir;
                            dir2.z = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(0.0f, 0.0f, 1.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = PI / (2.0f * res2);
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(1.0f, 0.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(cos(a2), sin(a2), 0.0f);
                            glm::vec3 d3(1.0f, 0.0f, 0.0f);
                            glm::vec3 d4(cos(a2), sin(a2), 0.0f);
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.z / 2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d2 - d1, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e3[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e3[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                            glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                            glm::vec3 norm(0.0f, 0.0f, -1.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        }
                    }
                    if (k == (dims[2] - 1)) {
                        v1 = nodes[i][j][k].e3[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                            glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                            glm::vec3 norm(0.0f, 0.0f, 1.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e3[0];
                    v2 = nodes[i][j][k].e3[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                        glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                        glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }
            }
        }
    }
    //v.insert(v.end(), tx.begin(), tx.end());
    //v.insert(v.end(), ty.begin(), ty.end());
    //v.insert(v.end(), tz.begin(), tz.end());
    //n.insert(v.end(), nx.begin(), nx.end());
    //n.insert(v.end(), ny.begin(), ny.end());
    //n.insert(v.end(), nz.begin(), nz.end());
    //c.insert(c.end(), cx.begin(), cx.end());
    //c.insert(c.end(), cy.begin(), cy.end());
    //c.insert(c.end(), cz.begin(), cz.end());

}

void generateStructureDataExp(Node***& nodes, int dims[3], std::vector<glm::vec3>& v, std::vector<glm::vec3>& c, std::vector<glm::vec3>& n, float radius, int res, float r, int res2, int axis, float angle) {
    v.clear();
    c.clear();
    n.clear();
    glm::vec3 v1;
    glm::vec3 v2;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if (axis & (1 << 0)) {
                    if (i != 0) {
                        int comp = ~(nodes[i - 1][j][k].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 0)) && (comp & (1 << 2))) {
                            v1 = nodes[i - 1][j][k].e1[1];
                            v2 = nodes[i][j][k].e1[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                        }
                        else {
                            v1 = nodes[i - 1][j][k].e1[1];
                            glm::vec3 dir = nodes[i][j][k].e1[0] - nodes[i - 1][j][k].e1[1];
                            glm::vec3 dir2 = dir;
                            dir2.x = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(1.0f, 0.0f, 0.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = angle / res2;
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(0.0f, 1.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(0.0f, cos(a2), sin(a2));
                            glm::vec3 d3(0.0f, 1.0f, 0.0f);
                            glm::vec3 d4(0.0f, cos(a2), sin(a2));
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.x / 2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d2 - d1, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    c.push_back(glm::vec3(1.0f, .0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e1[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                                glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                                c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e1[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                            glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                            glm::vec3 norm(-1.0f, 0.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        }
                    }
                    if (i == (dims[0] - 1)) {
                        v1 = nodes[i][j][k].e1[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                            glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                            glm::vec3 norm(1.0f, 0.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                            c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e1[0];
                    v2 = nodes[i][j][k].e1[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(0.0f, glm::cos(a1), glm::sin(a1));
                        glm::vec3 d2(0.0f, glm::cos(a2), glm::sin(a2));
                        glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                        c.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
                    }
                }
                if (axis & (1 << 1)) {
                    if (j != 0) {
                        int comp = ~(nodes[i][j - 1][k].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 0)) && (comp & (1 << 1))) {
                            v1 = nodes[i][j - 1][k].e2[1];
                            v2 = nodes[i][j][k].e2[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                        }
                        else {
                            v1 = nodes[i][j - 1][k].e2[1];
                            glm::vec3 dir = nodes[i][j][k].e2[0] - nodes[i][j - 1][k].e2[1];
                            glm::vec3 dir2 = dir;
                            dir2.y = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(0.0f, 1.0f, 0.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = angle /  res2;
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(1.0f, 0.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(cos(a2), 0.0f, sin(a2));
                            glm::vec3 d3(1.0f, 0.0f, 0.0f);
                            glm::vec3 d4(cos(a2), 0.0f, sin(a2));
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.y / 2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d1 - d2, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d1 - d2, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d1 - d2, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e2[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                                glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                                glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                                c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e2[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                            glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                            glm::vec3 norm(0.0f, -1.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }
                    if (j == (dims[1] - 1)) {
                        v1 = nodes[i][j][k].e2[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                            glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                            glm::vec3 norm(0.0f, 1.0f, 0.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                            c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e2[0];
                    v2 = nodes[i][j][k].e2[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(glm::cos(a1), 0.0f, glm::sin(a1));
                        glm::vec3 d2(glm::cos(a2), 0.0f, glm::sin(a2));
                        glm::vec3 normal = glm::normalize(glm::cross(v1 - v2, d2 - d1));
                        glm::vec3 norm(glm::cross(v1 - v2, d1 - d2));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        c.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                }
                if (axis & (1 << 2)) {
                    if (k != 0) {
                        int comp = ~(nodes[i][j][k - 1].config ^ nodes[i][j][k].config);
                        if ((comp & (1 << 1)) && (comp & (1 << 2))) {
                            v1 = nodes[i][j][k - 1].e3[1];
                            v2 = nodes[i][j][k].e3[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                        }
                        else {
                            v1 = nodes[i][j][k - 1].e3[1];
                            glm::vec3 dir = nodes[i][j][k].e3[0] - nodes[i][j][k - 1].e3[1];
                            glm::vec3 dir2 = dir;
                            dir2.z = 0;
                            float r1 = 2.0f * PI * r / (4.0f * res2);
                            glm::vec3 ax(0.0f, 0.0f, 1.0f);
                            glm::vec3 rotAx = glm::cross(ax, dir);
                            float a1 = angle / res2;
                            glm::mat3x3 R = rotationMatrix(rotAx, -a1);
                            glm::vec3 d1(1.0f, 0.0f, 0.0f);
                            float a2 = 2.0f * PI / res;
                            glm::vec3 d2(cos(a2), sin(a2), 0.0f);
                            glm::vec3 d3(1.0f, 0.0f, 0.0f);
                            glm::vec3 d4(cos(a2), sin(a2), 0.0f);
                            d1 = radius * d1;
                            d2 = radius * d2;

                            v2 = v1 + ax * (dir.z / 2.0f - r);
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            R = rotationMatrix(rotAx, a1);
                            v2 += ax * (glm::length(dir2) - 2.0f * r);
                            glm::mat3x3 R1 = rotationMatrix(ax, a2);
                            for (int m = 0; m < res; m++) {
                                glm::vec3 norm(glm::cross(d2 - d1, ax));
                                v.push_back(v1 + d1);
                                v.push_back(v1 + d2);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d1);
                                v.push_back(v2 + d2);
                                v.push_back(v1 + d2);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                d1 = R1 * d1;
                                d2 = R1 * d2;
                            }
                            v1 = v2;
                            for (int l = 0; l < res2; l++) {
                                v2 = v1 + R * ax * r1;
                                glm::mat3x3 R1 = rotationMatrix(ax, a2);
                                ax = R * ax;
                                for (int m = 0; m < res; m++) {
                                    d3 = R * d1;
                                    d4 = R * d2;
                                    glm::vec3 norm(glm::cross(d2 - d1, ax));
                                    v.push_back(v1 + d1);
                                    v.push_back(v1 + d2);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d3);
                                    v.push_back(v2 + d4);
                                    v.push_back(v1 + d2);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    n.push_back(norm);
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                    d1 = R1 * d1;
                                    d2 = R1 * d2;
                                }
                                d1 = d3;
                                d2 = d4;
                                v1 = v2;
                            }
                            v2 = nodes[i][j][k].e3[0];
                            for (int l = 0; l < res; l++) {
                                float a1 = l * 2.0f * PI / res;
                                float a2 = (l + 1) * 2.0f * PI / res;
                                glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                                glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                                glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                n.push_back(norm);
                                v.push_back(v1 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v2 + radius * d1);
                                v.push_back(v1 + radius * d2);
                                v.push_back(v2 + radius * d2);
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                                c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            }
                        }
                    }
                    else {
                        v1 = nodes[i][j][k].e3[0];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                            glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                            glm::vec3 norm(0.0f, 0.0f, -1.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        }
                    }
                    if (k == (dims[2] - 1)) {
                        v1 = nodes[i][j][k].e3[1];
                        for (int l = 0; l < res; l++) {
                            float a1 = l * 2.0f * PI / res;
                            float a2 = (l + 1) * 2.0f * PI / res;
                            glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                            glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                            glm::vec3 norm(0.0f, 0.0f, 1.0f);
                            n.push_back(norm);
                            n.push_back(norm);
                            n.push_back(norm);
                            v.push_back(v1 + radius * d1);
                            v.push_back(v1 + radius * d2);
                            v.push_back(v1);
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                            c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        }
                    }

                    v1 = nodes[i][j][k].e3[0];
                    v2 = nodes[i][j][k].e3[1];
                    for (int l = 0; l < res; l++) {
                        float a1 = l * 2.0f * PI / res;
                        float a2 = (l + 1) * 2.0f * PI / res;
                        glm::vec3 d1(glm::cos(a1), glm::sin(a1), 0.0f);
                        glm::vec3 d2(glm::cos(a2), glm::sin(a2), 0.0f);
                        glm::vec3 norm(glm::cross(v1 - v2, d2 - d1));
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        n.push_back(norm);
                        v.push_back(v1 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v2 + radius * d1);
                        v.push_back(v1 + radius * d2);
                        v.push_back(v2 + radius * d2);
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                        c.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }
            }
        }
    }
}

void exportObj(std::vector<glm::vec3>& p, std::vector<glm::vec3>& n, std::string name) {
    std::ostringstream ossp;
    std::ofstream file;
    file.open(name);

    //ossp << "# vertices" << "\n";
    for (int i = 0; i < p.size(); i++) {
        ossp << "v " << p[i].x << " " << p[i].y << " " << p[i].z << " " << "1.0" << "\n";
    }
    //ossp << "\n" << "# normals" << "\n";
    for (int i = 0; i < n.size(); i++) {
        ossp << "vn " << n[i].x << " " << n[i].y << " " << n[i].z << "\n";
    }
    //ossp << "\n" << "# faces" << "\n";
    for (int i = 1; i < n.size() + 1; i++) {
        ossp << "f " << i << "//" << i << " " << i + 1 << "//" << i + 1 << " " << i + 2 << "//" << i + 2 << "\n";
        i += 2;
    }
    file << ossp.str();
    file.close();
}
void exportSkeletonObj(Node***& nodes, int dims[3]) {
    std::ofstream file;
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> edgesx;
    std::vector<glm::vec3> edgesy;
    std::vector<glm::vec3> edgesz;
    for (int i = 0; i < dims[0]; i++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int k = 0; k < dims[2]; k++) {
                if (i != 0) {
                    edgesx.push_back(nodes[i - 1][j][k].e1[1]);
                    edgesx.push_back(nodes[i][j][k].e1[0]);
                }
                if (j != 0) {
                    edgesy.push_back(nodes[i][j - 1][k].e2[1]);
                    edgesy.push_back(nodes[i][j][k].e2[0]);
                }
                if (k != 0) {
                    edgesz.push_back(nodes[i][j][k - 1].e3[1]);
                    edgesz.push_back(nodes[i][j][k].e3[0]);
                }
                edgesx.push_back(nodes[i][j][k].e1[0]);
                edgesx.push_back(nodes[i][j][k].e1[1]);
                edgesy.push_back(nodes[i][j][k].e2[0]);
                edgesy.push_back(nodes[i][j][k].e2[1]);
                edgesz.push_back(nodes[i][j][k].e3[0]);
                edgesz.push_back(nodes[i][j][k].e3[1]);
            }
        }
    }
    v.insert(v.end(), edgesx.begin(), edgesx.end());
    v.insert(v.end(), edgesy.begin(), edgesy.end());
    v.insert(v.end(), edgesz.begin(), edgesz.end());
    file.open("skeletonExport.obj");
    for (int i = 0; i < v.size(); i++) {
        file << "v " << v[i].x << " " << v[i].y << " " << v[i].z << " 1.0" << "\n";
    }
    for (int i = 0; i < v.size(); i++) {
        file << "l " << i + 1 << " " << i + 2 << "\n";
        i += 1;
    }
    file.close();
}

void generateGridData(std::vector<glm::vec3>& gridVertices, int a) {
    gridVertices.clear();
    glm::vec3 orig = glm::vec3(-a/2.0f, -a/2.0f, -2.0f);
    int sq = a * a;
    for (int i = 0; i < sq; i++) {
        glm::vec3 p1(i % a, i / a, 0.0f);
        glm::vec3 p2(i % a + 1, i / a, 0.0f);
        glm::vec3 p3(i % a, i / a + 1, 0.0f);
        glm::vec3 p4(i % a + 1, i / a + 1, 0.0f);

        gridVertices.push_back(orig + p1);
        gridVertices.push_back(orig + p2);

        gridVertices.push_back(orig + p1);
        gridVertices.push_back(orig + p3);

        gridVertices.push_back(orig + p2);
        gridVertices.push_back(orig + p4);

        gridVertices.push_back(orig + p3);
        gridVertices.push_back(orig + p4);
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

    float radius = 0.3f;
    int res = 16;
    int res1 = 8;
    float rad2 = 0.1f;
    std::vector<glm::vec3> v;
    std::vector<glm::vec3> c;
    std::vector<glm::vec3> n;
    int axis = 7;
    generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
    unsigned int vao, vbo_pos, vbo_col, vbo_norm;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_pos);
    glGenBuffers(1, &vbo_col);
    glGenBuffers(1, &vbo_norm);

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

    glBindBuffer(GL_ARRAY_BUFFER, vbo_norm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * n.size(), &n[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    unsigned int VAO_plane;
    glGenVertexArrays(1, &VAO_plane);

    float axisVertices[] = {
        0.0f,0.0f,0.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,
        0.0f,0.0f,1.0f,

        0.0f,0.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,
        0.0f,1.0f,0.0f,

        0.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f,0.0f,0.0f
    };

    unsigned int VBO_pos_p;
    glGenBuffers(1, &VBO_pos_p);

    glBindVertexArray(VAO_plane);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos_p);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), &axisVertices[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    std::vector<glm::vec3> gridVertices;
    glm::vec3 orig = glm::vec3(-8.0f, -8.0f, -2.0f);
    for (int i = 0; i < 256; i++) {
        glm::vec3 p1(i % 16, i / 16, 0.0f);
        glm::vec3 p2(i % 16 + 1, i / 16, 0.0f);
        glm::vec3 p3(i % 16, i / 16 + 1, 0.0f);
        glm::vec3 p4(i % 16 + 1, i / 16 + 1, 0.0f);

        gridVertices.push_back(orig + p1);
        gridVertices.push_back(orig + p2);

        gridVertices.push_back(orig + p1);
        gridVertices.push_back(orig + p3);

        gridVertices.push_back(orig + p2);
        gridVertices.push_back(orig + p4);

        gridVertices.push_back(orig + p3);
        gridVertices.push_back(orig + p4);
    }

    unsigned int VAO_grid;
    glGenVertexArrays(1, &VAO_grid);

    unsigned int VBO_grid;
    glGenBuffers(1, &VBO_grid);

    glBindVertexArray(VAO_grid);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_grid);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * 3 * sizeof(float), &gridVertices[0], GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int width, height;
    Shader shader = Shader("C:\\Src\\shaders\\linevertex.glsl", "C:\\Src\\shaders\\linefrag.glsl", "C:\\Src\\shaders\\cylinder.glsl");
    Shader lineshader = Shader("C:\\Src\\shaders\\vertCoss.glsl", "C:\\Src\\shaders\\fragCoss.glsl");
    Shader pipeShader = Shader("C:\\Src\\shaders\\vertNorm.glsl", "C:\\Src\\shaders\\fragNorm.glsl");

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
    float orthoScale = 10.0f, angle = 30.0;
    glm::vec3 lightPos = { 10.0f,10.f, 10.f };
    bool xz = true, yz = true, xy = true, abc = false, x = true, y = true, z = true, ax = true, grid = true, inc = false, inc2 = false;
    int config = 7, gridSize = 8;
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
        //glBindVertexArray(vao);
        //glDrawArrays(GL_LINES, 0, v.size());
        
        pipeShader.use();
        pipeShader.setVec3("lightPos", lightPos);
        pipeShader.setMat4("projection", projection);
        pipeShader.setMat4("view", view);
        pipeShader.setMat4("model", model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, v.size());
        
        lineshader.use();
        lineshader.setMat4("projection", projection);
        lineshader.setMat4("view", view);
        lineshader.setMat4("model", model);
        if (grid) {
            glBindVertexArray(VAO_grid);
            glDrawArrays(GL_LINES, 0, gridVertices.size());
        }
        if (ax) {
            glBindVertexArray(VAO_plane);
            glDrawArrays(GL_LINES, 0, 12);
        }
        
        //start of imgui init stuff
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
        ImGui::Begin("Structure Settings");
        if (ImGui::DragInt3("Dimension", dims, .2f, 2, 100)) {
            if (dims[0] == 0)
                dims[0] = 1;
            if (dims[1] == 0)
                dims[1] = 1;
            if (dims[2] == 0)
                dims[2] = 1;
            if (dims[0] > 50)
                dims[0] = 50;
            if (dims[1] > 50)
                dims[1] = 50;
            if (dims[2] > 50)
                dims[2] = 50;
            lightPos[0] = dims[0] * 10.0f;
            lightPos[1] = dims[1] * 10.0f;
            lightPos[2] = dims[2] * 10.0f;

            if (abc)
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            else if (inc)
                generateIncStructure(allNodes, dims, space, config);
            else if (inc2)
                generateInc2Structure(allNodes, dims, space, config);
            else
                generateRandomStructure(allNodes, dims, space, config);

            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragFloat("Space", &space, 0.1f, 0.01f, 100.0f)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragFloat("Pipe Angle", &angle, 0.1f, -360.0f, 360.0f)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis, glm::radians(angle));
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Enable XY Mirror", &xy)) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (abc)
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            else if(inc)
                generateIncStructure(allNodes, dims, space, config);
            else if (inc2)
                generateInc2Structure(allNodes, dims, space, config);
            else
                generateRandomStructure(allNodes, dims, space, config);

            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Enable XZ Mirror", &xz) ) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (abc)
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            else if (inc)
                generateIncStructure(allNodes, dims, space, config);
            else if (inc2)
                generateInc2Structure(allNodes, dims, space, config);
            else
                generateRandomStructure(allNodes, dims, space, config);

            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Enable YZ Mirror", &yz)) {
            config = xy * 1 + yz * 2 + xz * 4;
            if (abc)
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            else if (inc)
                generateIncStructure(allNodes, dims, space, config);
            else if (inc2)
                generateInc2Structure(allNodes, dims, space, config);
            else
                generateRandomStructure(allNodes, dims, space, config);

            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("ABC", &abc)) {
            inc = false;
            inc2 = false;
            generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (abc) {
            if (ImGui::DragInt3("ABC", ABC, .2f, 1, 70)) {
                if (ABC[0] == 0)
                    ABC[0] = 1;
                if (ABC[1] == 0)
                    ABC[1] = 1;
                generateABCStructure(allNodes, dims, space, config, ABC[0], ABC[1], ABC[2]);
                generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
                updateBufferData(vbo_pos, v);
                updateBufferData(vbo_col, c);
                updateBufferData(vbo_norm, n);
            }
        }
        if (ImGui::Checkbox("Inc", &inc)) {
            abc = false;
            inc2 = false;
            generateIncStructure(allNodes, dims, space, config);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Inc2", &inc2)) {
            abc = false;
            inc = false;
            generateInc2Structure(allNodes, dims, space, config);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        
        if (ImGui::Checkbox("X Strands", &x)) {
            axis = x * 1 + y * 2 + z * 4;
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Y Strands", &y)) {
            axis = x * 1 + y * 2 + z * 4;
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::Checkbox("Z Strands", &z)) {
            axis = x * 1 + y * 2 + z * 4;
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragFloat("Radius", &radius, 0.01f)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragInt("Pipe Res", &res, 0.1f, 2)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragFloat("Corner Radius", &rad2, 0.05f, 0.01f, space/4.0f)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        if (ImGui::DragInt("Corner Res", &res1, 0.1f, 2)) {
            updateStructure(allNodes, dims, space);
            generateStructureDataExp(allNodes, dims, v, c, n, radius, res, rad2, res1, axis);
            updateBufferData(vbo_pos, v);
            updateBufferData(vbo_col, c);
            updateBufferData(vbo_norm, n);
        }
        ImGui::End();

        ImGui::Begin("Render Settings");
        ImGui::DragFloat3("Light Pos", &lightPos[0], 0.05);
        ImGui::Checkbox("Orthographic", &ortho);
        if(ortho)
            ImGui::DragFloat("Orthographic Scale", &orthoScale, 0.05, 0.01f, 100.0f);
        ImGui::Checkbox("Grid", &grid);
        
        if (grid) {
            if (ImGui::DragInt("Size", &gridSize), 1, 100) {
                generateGridData(gridVertices, gridSize);
                updateBufferData(VBO_grid, gridVertices);
            }
        }
        ImGui::Checkbox("Axis", &ax);
        if (ImGui::Button("Export OBJ"))
            exportObj(v, n, "export.obj");
        if (ImGui::Button("Export Skeleton OBJ"))
            exportSkeletonObj(allNodes, dims);
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

