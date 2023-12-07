#include "movement.h"
#include <iostream>

Movement::Movement(Camera& camera) : cam_(camera)
{

}

void Movement::forward(float dist) {
    glm::vec3 updated = cam_.getPos();
    glm::vec3 og = updated;
    updated += dist * glm::normalize(glm::vec3(cam_.getLook()));
    cam_.updatePos(updated);
    std::cout << og.x << ","<< og.y << ","<< og.z << " to "<< updated.x << ","<< updated.y << ","<< updated.z << std::endl;
}
void Movement::backward(float dist) {
    glm::vec3 updated = cam_.getPos();
    updated -= dist * glm::normalize(glm::vec3(cam_.getLook()));
    cam_.updatePos(updated);
}
void Movement::left(float dist) {
    glm::vec3 updated = cam_.getPos();
    updated -= dist * glm::normalize(glm::cross(glm::vec3(cam_.getLook()),glm::vec3(cam_.getUp())));
    cam_.updatePos(updated);
}
void Movement::right(float dist) {
    glm::vec3 updated = cam_.getPos();
    updated += dist * glm::normalize(glm::cross(glm::vec3(cam_.getLook()),glm::vec3(cam_.getUp())));
    cam_.updatePos(updated);
}
void Movement::up(float dist) {
    glm::vec3 updated = cam_.getPos();
    glm::vec3 axis = glm::vec3(cam_.getInverseViewMatrix() * glm::vec4(0,1,0,0));
    updated += dist * glm::normalize(axis);
    cam_.updatePos(updated);
}
void Movement::down(float dist) {
    glm::vec3 updated = cam_.getPos();
    glm::vec3 axis = glm::vec3(cam_.getInverseViewMatrix() * glm::vec4(0,1,0,0));
    updated -= dist * glm::normalize(axis);
    cam_.updatePos(updated);
}
