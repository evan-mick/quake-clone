#include "cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<GLfloat>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}


void Cone::makeSlice(float currentTheta, float nextTheta) {

    for(int c=0;c<m_param1;c++) {
        float cur_radius = c * static_cast<float>(0.5) / static_cast<float>(m_param1);
        float next_radius = (c+1) * static_cast<float>(0.5) / static_cast<float>(m_param1);

        float topY = 0.5 - (c / static_cast<float>(m_param1));
        float bottomY = 0.5 - ((c +1)/ static_cast<float>(m_param1));
        glm::vec3 topLeft =  glm::vec3(cur_radius * sin(currentTheta),topY,cur_radius * cos(currentTheta));
        glm::vec3 topRight = glm::vec3(cur_radius * sin(nextTheta),topY,cur_radius * cos(nextTheta));
        glm::vec3 bottomLeft = glm::vec3(next_radius * sin(currentTheta),bottomY,next_radius * cos(currentTheta));
        glm::vec3 bottomRight = glm::vec3(next_radius * sin(nextTheta),bottomY,next_radius * cos(nextTheta));
        glm::vec3 norm1 = glm::vec3(next_radius * sin(currentTheta),next_radius*0.5,next_radius * cos(currentTheta));
        glm::vec3 norm2 = glm::vec3(next_radius * sin(nextTheta),next_radius*0.5,next_radius * cos(nextTheta));
        if(c==0) {
            float midTheta = (currentTheta + nextTheta) * 0.5;
            glm::vec3 norm3 = next_radius * glm::vec3(sin(midTheta),0.5,cos(midTheta));
            insertVec3(m_vertexData,topLeft);
            insertVec3(m_vertexData,norm3);

            insertVec3(m_vertexData,bottomLeft);
            insertVec3(m_vertexData,norm1);

            insertVec3(m_vertexData,bottomRight);
            insertVec3(m_vertexData,norm2);

        } else {
            insertVec3(m_vertexData,topRight);
            insertVec3(m_vertexData,norm2);

            insertVec3(m_vertexData,topLeft);
            insertVec3(m_vertexData,norm1);

            insertVec3(m_vertexData,bottomLeft);
            insertVec3(m_vertexData,norm1);


            insertVec3(m_vertexData,bottomRight);
            insertVec3(m_vertexData,norm2);

            insertVec3(m_vertexData,topRight);
            insertVec3(m_vertexData,norm2);

            insertVec3(m_vertexData,bottomLeft);
            insertVec3(m_vertexData,norm1);
        }
    }
}

void Cone::makeBaseSlice(float currentTheta, float nextTheta) {
    glm::vec3 normal(0,-1,0);
    double rStep = 0.5/m_param1;
    for(double r=0.0;r+rStep<=0.5;r+=rStep) {
        glm::vec3 topLeft(r * sin(currentTheta),-0.5,r*cos(currentTheta));
        glm::vec3 topRight(r * sin(nextTheta),-0.5,r*cos(nextTheta));
        glm::vec3 bottomLeft((r+rStep) * sin(currentTheta),-0.5,(r+rStep)*cos(currentTheta));
        glm::vec3 bottomRight((r+rStep) * sin(nextTheta),-0.5,(r+rStep)*cos(nextTheta));

        insertVec3(m_vertexData,topRight);
        insertVec3(m_vertexData,normal);

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
    }
}

void Cone::makeCone() {
    float thetaStep = glm::radians(360.f / m_param2);
    float theta = 0.f;
    float nextTheta = thetaStep;
    while(nextTheta<=2*M_PI+.1) {
        makeSlice(theta,nextTheta);
        makeBaseSlice(theta,nextTheta);
        theta = nextTheta;
        nextTheta = nextTheta + thetaStep;
    }
}

void Cone::setVertexData() {
    makeCone();
}


void Cone::insertVec3(std::vector<GLfloat> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
