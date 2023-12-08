#include "sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<GLfloat>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, glm::normalize(topLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, glm::normalize(bottomRight));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, glm::normalize(topRight));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, glm::normalize(bottomLeft));
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float phiStep = M_PI/static_cast<float>(m_param1);

    glm::vec3 topLeft(0.5 * glm::sin(0) * sin(currentTheta),
                      0.5*cos(0),
                      0.5*sin(0)*cos(currentTheta));
    glm::vec3 topRight (0.5 * glm::sin(0) * sin(nextTheta),
                       0.5*cos(0),
                       0.5*sin(0)*cos(nextTheta));
    glm::vec3 bottomLeft(0.5 * glm::sin(phiStep) * sin(currentTheta),
                         0.5*cos(phiStep),
                         0.5*sin(phiStep)*cos(currentTheta));
    glm::vec3 bottomRight(0.5 * glm::sin(phiStep) * sin(nextTheta),
                          0.5*cos(phiStep),
                          0.5*sin(phiStep)*cos(nextTheta));
    makeTile(topLeft,topRight,bottomLeft,bottomRight);


    for(float phi=phiStep;phi<=M_PI-phiStep+.1;phi=phi+phiStep) {
        topLeft = bottomLeft;
        topRight = bottomRight;

        bottomLeft = glm::vec3(0.5 * glm::sin(phi+phiStep) * sin(currentTheta),
                               0.5 * cos(phi+phiStep),
                               0.5 * glm::sin(phi+phiStep)*cos(currentTheta));
        if(phi+phiStep>=M_PI-.1&&phi+phiStep<=M_PI+.1){
            bottomLeft = glm::vec3(0,-.5,0);
            bottomRight = bottomLeft;
        } else {
            bottomRight = glm::vec3(0.5 * glm::sin(phi+phiStep) * sin(nextTheta),
                                    0.5*cos(phi+phiStep),
                                    0.5*sin(phi+phiStep)*cos(nextTheta));
        }
        makeTile(topLeft,topRight,bottomLeft,bottomRight);
    }
}

void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / m_param2);
    float theta = 0.f;
    float nextTheta = thetaStep;
    while(nextTheta<=2*M_PI+.1) {
        makeWedge(theta,nextTheta);
        theta = nextTheta;
        nextTheta = nextTheta + thetaStep;
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

void Sphere::insertVec3(std::vector<GLfloat> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
