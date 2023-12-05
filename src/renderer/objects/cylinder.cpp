#include "cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<GLfloat>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    for(int c=0;c<m_param1;c++) {
        float radius = 0.5;

        float topY = 0.5 - (c / static_cast<float>(m_param1));
        float bottomY = 0.5 - ((c +1)/ static_cast<float>(m_param1));
        glm::vec3 topLeft =  glm::vec3(radius * sin(currentTheta),topY,radius * cos(currentTheta));
        glm::vec3 topRight = glm::vec3(radius * sin(nextTheta),topY,radius * cos(nextTheta));
        glm::vec3 bottomLeft = glm::vec3(radius * sin(currentTheta),bottomY,radius * cos(currentTheta));
        glm::vec3 bottomRight = glm::vec3(radius * sin(nextTheta),bottomY,radius * cos(nextTheta));
        glm::vec3 norm1 = glm::vec3(radius * sin(currentTheta),0,radius * cos(currentTheta));
        glm::vec3 norm2 = glm::vec3(radius * sin(nextTheta),0,radius * cos(nextTheta));

        insertVec3(m_vertexData,topLeft);
        insertVec3(m_vertexData,norm1);

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,norm1);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,norm2);

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,norm1);

        insertVec3(m_vertexData,bottomRight);
        insertVec3(m_vertexData,norm2);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,norm2);
    }

    makeSlice(currentTheta,nextTheta);
    makeTopSlice(currentTheta,nextTheta);
}

void Cylinder::makeTopSlice(float currentTheta, float nextTheta) {
    glm::vec3 normal(0,1,0);
    float y = 0.5f;

    double rStep = 0.5/m_param1;
    for(double r=0.0;r+rStep<=0.5;r+=rStep) {
        glm::vec3 topLeft(r * sin(currentTheta),y,r*cos(currentTheta));
        glm::vec3 topRight(r * sin(nextTheta),y,r*cos(nextTheta));
        glm::vec3 bottomLeft((r+rStep) * sin(currentTheta),y,(r+rStep)*cos(currentTheta));
        glm::vec3 bottomRight((r+rStep) * sin(nextTheta),y,(r+rStep)*cos(nextTheta));

        insertVec3(m_vertexData,topLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,bottomRight);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,normal);


    }
}

void Cylinder::makeSlice(float currentTheta, float nextTheta) {
    glm::vec3 normal(0,-1,0);
    float y = -0.5;

    double rStep = 0.5/m_param1;
    for(double r=0.0;r+rStep<=0.5;r+=rStep) {
        glm::vec3 topLeft(r * sin(currentTheta),y,r*cos(currentTheta));
        glm::vec3 topRight(r * sin(nextTheta),y,r*cos(nextTheta));
        glm::vec3 bottomLeft((r+rStep) * sin(currentTheta),y,(r+rStep)*cos(currentTheta));
        glm::vec3 bottomRight((r+rStep) * sin(nextTheta),y,(r+rStep)*cos(nextTheta));

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,topLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,bottomRight);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,bottomLeft);
        insertVec3(m_vertexData,normal);

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,normal);


    }
}

void Cylinder::makeCylinder() {
    float thetaStep = glm::radians(360.f / m_param2);
    float theta = 0.f;
    float nextTheta = thetaStep;
    while(nextTheta<=2*M_PI+.1) {
        makeWedge(theta,nextTheta);
        theta = nextTheta;
        nextTheta = nextTheta + thetaStep;
    }
}

void Cylinder::setVertexData() {
    makeCylinder();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<GLfloat> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
