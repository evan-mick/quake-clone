#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GL/glew.h"

class Cone
{
public:
    void updateParams(int param1, int param2);
    std::vector<GLfloat> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<GLfloat> &data, glm::vec3 v);
    void setVertexData();
    void makeCone();
    void makeBaseSlice(float currentTheta, float nextTheta);
    void makeSlice(float currentTheta, float nextTheta);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;
};
