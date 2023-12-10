#include <stdexcept>
#include "camera.h"
#include <iostream>
#include "glm/gtx/transform.hpp""


Camera::Camera(int width, int height, const SceneCameraData& cameraData) {
    focalLength_ = cameraData.focalLength;
    aperture_ = cameraData.aperture;
    heightAngle_ = cameraData.heightAngle;
    viewMatrix_ = Camera::calculateViewMatrix(glm::vec3(cameraData.pos),
                                              glm::vec3(cameraData.look),
                                              glm::vec3(cameraData.up));
    look_ = cameraData.look;
    up_ = cameraData.up;
    inverseViewMatrix_ = glm::inverse(viewMatrix_);
    aspectRatio_ = static_cast<float>(width)/static_cast<float>(height);
    pos_ = cameraData.pos;

    projMat_ = getPerspectiveMatrix(0.1f, 100);
}

Camera::Camera() {
    focalLength_ = 0.f;
    aperture_ = 0.f;
    heightAngle_ = 0.f;
    viewMatrix_ = glm::mat4(1.f);
    inverseViewMatrix_ = glm::mat4(1.f);
    aspectRatio_ = 0.f;
    pos_ = glm::vec3(0.f);
}

glm::mat4 Camera::calculateViewMatrix(glm::vec3 pos,glm::vec3 look, glm::vec3 up) {
    glm::mat4 t = glm::mat4(1.0f);

    t[3][0] = -pos.x;
    t[3][1] = -pos.y;
    t[3][2] = -pos.z;
    glm::vec3 w = glm::normalize(-look);
    glm::vec3 v = glm::normalize(up - glm::dot(glm::vec3(up),w)*w);
    glm::vec3 u = glm::cross(v,w);

    glm::mat4 rotate = glm::mat4(
        u.x,v.x,w.x,0.f,
        u.y,v.y,w.y,0.f,
        u.z,v.z,w.z,0.f,
        0.f, 0.f, 0.f, 1.f);

    return rotate * t;
}

void Camera::updateRotation(float dX, float dY) {
    look_ = rotateCamX(dX) * rotateCamY(dY) * look_;
    viewMatrix_ = calculateViewMatrix(pos_,glm::vec3(look_),glm::vec3(up_));
    inverseViewMatrix_ = glm::inverse(viewMatrix_);
}

void Camera::setRotation(float x, float y) {
//    look_ = rotateCamX(x) * rotateCamY(y) * glm::vec4(0, 0, 0, 1);


    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), x, glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, y, glm::vec3(0.0f, 0.0f, 1.0f));

    look_ = rotationMatrix * glm::vec4(1, 1, 1, 1);

    viewMatrix_ = calculateViewMatrix(pos_,glm::vec3(look_),glm::vec3(up_));
    inverseViewMatrix_ = glm::inverse(viewMatrix_);
}

void Camera::updatePos(glm::vec3 updated) {
    pos_ = updated;
    viewMatrix_ = calculateViewMatrix(pos_,glm::vec3(look_),glm::vec3(up_));
    inverseViewMatrix_ = glm::inverse(viewMatrix_);
}

glm::mat4 rotateAboutAxis(glm::vec3 axis, float angle) {
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    float oneMinusCosTheta = 1.0f - cosTheta;

    glm::mat3 crossProductMatrix = glm::mat3(0, -axis.z, axis.y,
                                             axis.z, 0, -axis.x,
                                             -axis.y, axis.x, 0); //axis components

    glm::mat3 rotationMatrix = cosTheta * glm::mat3(1.0f) +
                               oneMinusCosTheta * glm::outerProduct(axis, axis) +
                               sinTheta * crossProductMatrix;

    glm::mat4 result = glm::mat4(1.0f);
    result[0] = glm::vec4(rotationMatrix[0], 0.0f);
    result[1] = glm::vec4(rotationMatrix[1], 0.0f);
    result[2] = glm::vec4(rotationMatrix[2], 0.0f);

    return result;
}

glm::mat4 Camera::rotateCamX(float dX) {
    glm::vec3 axis = glm::normalize(glm::vec3(inverseViewMatrix_ * glm::vec4(0,1,0,0)));
    float angle = glm::radians(dX*sensitivityX);
    glm::mat4 rot = rotateAboutAxis(axis,angle);
    return rot;
}

glm::mat4 Camera::rotateCamY(float dY) {
    glm::vec3 axis = glm::normalize(glm::cross(glm::vec3(up_),glm::vec3(look_)));
    float angle = glm::radians(-dY*sensitivityY);
    glm::mat4 rot = rotateAboutAxis(axis,angle);
    return rot;
}

glm::mat4 Camera::getPerspectiveMatrix(float near, float far) {
    float c = -1 * near /far;
    glm::mat4 coeff(1,0,0,0,
                    0,1,0,0,
                    0,0,-2,0,
                    0,0,-1,1);
    glm::mat4 mpp(1,0,0,0,
                  0,1,0,0,
                  0,0,1.0/(1+c),-1,
                  0,0,-c/(1+c),0);

    float sx = 1.f / (far * glm::tan(heightAngle_ * aspectRatio_ * 0.5));
    float sy = 1.f / (far * glm::tan(heightAngle_ * 0.5));
    float sz = 1.f / far;
    glm::mat4 viewVolume(sx,0,0,0,
                         0,sy,0,0,
                         0,0,sz,0,
                         0,0,0,1);
    return coeff * mpp * viewVolume;
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix_;
}

glm::mat4 Camera::getInverseViewMatrix() const {
    return inverseViewMatrix_;
}

float Camera::getAspectRatio() const {
    return aspectRatio_;
}

float Camera::getHeightAngle() const {
    return heightAngle_;
}

float Camera::getFocalLength() const {
    return focalLength_;
}

float Camera::getAperture() const {
    return aperture_;
}

glm::vec3 Camera::getPos() const {
    return pos_;
}

glm::vec4 Camera::getLook() const {
    return look_;
}

glm::vec4 Camera::getUp() const {
    return up_;
}
