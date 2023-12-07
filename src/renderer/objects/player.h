#ifndef PLAYER_H
#define PLAYER_H

#include "renderer/scenedata.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#define PRIM_COUNT 7
#define HEAD_I 0
#define NECK_I 1
#define BODY_I 2
#define LEFT_LEG_I 3
#define RIGHT_LEG_I 4
#define LEFT_ARM_I 5
#define RIGHT_ARM_I 6
#include "renderer/rendermodel.h"

class Player
{
public:
    Player();
    void assignModelID(u_int8_t id);
    Model getModel();
    void generateGeometry();
    void startAnimation();
    void stepLegs(float deltaTime);
    void stepArms(float angle);
    void stopAnimation();
    void rotatePlayer(float angle, glm::vec3 axis);
    void translatePlayer(glm::vec3 delta);
    void relocatePlayer(glm::vec3 position);
    std::array<RenderObject,PRIM_COUNT> m_geometry;
    float m_leg_angle = 0;
private:
    void makeHead(glm::mat4 root_ctm);
    void makeLegs(glm::mat4 root_ctm);
    void makeBody(glm::mat4 root_ctm);
    void makeNeck(glm::mat4 body_ctm);
    void makeArms(glm::mat4 body_ctm);
    void insertGeometry(glm::mat4 ctm, PrimitiveType type, int index);


    glm::mat4 m_root_ctm;

    bool loaded;
    bool walking;
    u_int8_t m_model_id = -1;

    glm::mat4 m_left_ctm;
    glm::mat4 m_right_ctm;

    glm::mat4 m_left_arm_ctm;
    glm::mat4 m_right_arm_ctm;

//    Renderable* m_left;
//    Renderable* m_right;
    const float WALK_SPEED = 100.f; //degrees per second
    int sign;
    SceneMaterial m_material;
};

#endif // PLAYER_H
