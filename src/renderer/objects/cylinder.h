#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
//#include "glfw/glfw3.h"

class Cylinder
{
public:
    void updateParams(int param1, int param2);
    std::vector<GLfloat> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void setVertexData();
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  glm::vec3 sideNormal);
    void makeWedge(float currTheta, float nextTheta);
    void makeCylinder();
    void makeSlice(float currentTheta, float nextTheta);
    void makeTopSlice(float currentTheta, float nextTheta);

    std::vector<float> m_vertexData;
    float m_radius = 0.5;
    int m_param1;
    int m_param2;
};
