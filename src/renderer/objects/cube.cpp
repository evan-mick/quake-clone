#include "cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    glm::vec3 normal) {

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
}

void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, glm::vec3 normal) {
    glm::vec3 horizStep = (topRight - topLeft) / static_cast<float>(m_param1);
    glm::vec3 vertStep = (bottomLeft - topLeft) / static_cast<float>(m_param1);

    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            glm::vec3 adjustedTL = topLeft + static_cast<float>(i) * horizStep + static_cast<float>(j) * vertStep;
            makeTile(adjustedTL,adjustedTL+horizStep,adjustedTL+vertStep,adjustedTL+horizStep+vertStep,normal);
        }
    }
}


void Cube::setVertexData() {
    glm::vec3 normal;
    normal = glm::vec3(0.0f, 0.0f, 1.0f); // Front face normal
    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f), normal);

    normal = glm::vec3(0.0f, 0.0f, -1.0f); // Back face normal
    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f), normal);

    normal = glm::vec3(0.0f, 1.0f, 0.0f); // Top face normal
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f), normal);

    normal = glm::vec3(0.0f, -1.0f, 0.0f); // Bottom face normal
    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f), normal);

    normal = glm::vec3(-1.0f, 0.0f, 0.0f); // Left face normal
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f), normal);

    normal = glm::vec3(1.0f, 0.0f, 0.0f); // Right face normal
    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f), normal);
}

void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
