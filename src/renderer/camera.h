#pragma once

#include <glm/glm.hpp>
#include "scenedata.h"

class Camera {
public:
    Camera(int width, int height, const SceneCameraData &cameraData);
    Camera();
    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;

    glm::mat4 getInverseViewMatrix() const;

    glm::mat4 getPerspectiveMatrix(float near, float far);

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;

    glm::vec3 getPos() const;

    glm::vec4 getLook() const;

    glm::vec4 getUp() const;

    void updatePos(glm::vec3 pos);
    void updateRotation(float dX, float dY);

    void rotateCam(float dX,float dY);
    glm::mat4 rotateCamX(float dX);
    glm::mat4 rotateCamY(float dY);
    glm::vec4 look_;
    glm::vec4 up_;
private:
    glm::mat4 viewMatrix_;
    glm::mat4 inverseViewMatrix_;
    float aspectRatio_;
    float heightAngle_;
    float focalLength_;
    float aperture_;
    static glm::mat4 calculateViewMatrix(glm::vec3 pos,glm::vec3 look, glm::vec3 up);
    glm::vec3 pos_;

    float sensitivityX = .3;
    float sensitivityY = .3;
};
