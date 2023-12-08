#include "player.h"
#include <iostream>


Player::Player()
{
    walking = false;
    loaded = false;
    sign = 1;
    m_root_ctm = glm::mat4(1.f);
    m_material.cAmbient = glm::vec4(0.5,0.5,0.5,1);
    m_material.cDiffuse = glm::vec4(1);
    m_material.cSpecular = glm::vec4(1);
    m_material.shininess = 1.0;
}

void Player::assignModelID(u_int8_t id) {
    m_model_id = id;
}

Model Player::getModel() {
    std::vector<RenderObject *> objects;
    for(int i=0;i<PRIM_COUNT;i++) {
        objects.push_back(&m_geometry[i]);
    }

    return {};
//    return (Model){.objects=objects,.id=m_model_id};
}

void Player::rotatePlayer(float angle, glm::vec3 axis) {
    m_root_ctm = glm::rotate(m_root_ctm,angle,axis);
    generateGeometry();
}

void Player::translatePlayer(glm::vec3 delta) {
    m_root_ctm = glm::translate(m_root_ctm,delta);
    generateGeometry();
}

void Player::relocatePlayer(glm::vec3 position) {
    m_root_ctm[3][0] = position[0];
    m_root_ctm[3][1] = position[1];
    m_root_ctm[3][2] = position[2];
}

void Player::generateGeometry() {
    glm::mat4 root = glm::translate(m_root_ctm,glm::vec3(0,0.3,0));
    makeHead(root);
    makeBody(root);
    makeLegs(root);
    loaded = true;
}

void Player::makeHead(glm::mat4 root_ctm) {
    glm::mat4 head_ctm = glm::translate(root_ctm,glm::vec3(0,2,0));
    head_ctm = glm::scale(head_ctm,glm::vec3(.9));
    insertGeometry(head_ctm,PrimitiveType::PRIMITIVE_SPHERE,HEAD_I);

}
void Player::makeLegs(glm::mat4 root_ctm) {
    glm::mat4 left = glm::translate(root_ctm,glm::vec3(0.15,0.2,0));
    left = glm::scale(left,glm::vec3(0.2,1,0.3));

    glm::mat4 right = glm::translate(root_ctm,glm::vec3(-0.15,0.2,0));
    right = glm::scale(right,glm::vec3(0.2,1,0.3));

    insertGeometry(left,PrimitiveType::PRIMITIVE_CUBE,LEFT_LEG_I);
    insertGeometry(right,PrimitiveType::PRIMITIVE_CUBE,RIGHT_LEG_I);

}
void Player::makeBody(glm::mat4 root_ctm) {
    glm::mat4 body_ctm = glm::translate(root_ctm,glm::vec3(0,1,0));
    body_ctm = glm::scale(body_ctm,glm::vec3(0.7,0.85,0.7));

    insertGeometry(body_ctm,PrimitiveType::PRIMITIVE_CYLINDER,BODY_I);

    makeNeck(body_ctm);
    makeArms(body_ctm);
}

void Player::makeNeck(glm::mat4 body_ctm) {
    glm::mat4 neck = glm::translate(body_ctm,glm::vec3(0,.85,0));
    neck = glm::scale(neck,glm::vec3(1,0.85,1));

    insertGeometry(neck,PrimitiveType::PRIMITIVE_CONE,NECK_I);
}

void Player::makeArms(glm::mat4 body_ctm) {
    glm::mat4 left = glm::translate(body_ctm,glm::vec3(0.55,.15,0));
    left = glm::scale(left,glm::vec3(0.15,0.7,0.15));

    glm::mat4 right = glm::translate(body_ctm,glm::vec3(-0.55,.15,0));
    right = glm::scale(right,glm::vec3(0.15,0.7,0.15));

    insertGeometry(left,PrimitiveType::PRIMITIVE_CUBE,LEFT_ARM_I);
    insertGeometry(right,PrimitiveType::PRIMITIVE_CUBE,RIGHT_ARM_I);
}


void Player::stepLegs(float deltaTime) {
    if(walking&&loaded) {
        float angle_change = deltaTime * WALK_SPEED;
        m_leg_angle += angle_change * sign;
        if(m_leg_angle > 30.01 || m_leg_angle < -30.01) {
            sign *= -1;
            m_leg_angle = m_leg_angle < 0 ? -30.f : 30.f;
        }
        float translation_y = 1;

        glm::mat4 rotatedLeft = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, translation_y, 0.0f))
                                * glm::rotate(glm::radians(m_leg_angle), glm::vec3(1, 0, 0))
                                * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -translation_y, 0.0f));

        glm::mat4 rotatedRight = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, translation_y, 0.0f))
                                 * glm::rotate(glm::radians(-m_leg_angle), glm::vec3(1, 0, 0))
                                 * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -translation_y, 0.0f));

        m_geometry[LEFT_LEG_I].ctm = rotatedLeft * m_left_ctm;
        m_geometry[RIGHT_LEG_I].ctm = rotatedRight * m_right_ctm;
        stepArms(-m_leg_angle / 2.f);
    }
}

void Player::stepArms(float angle) {
    if(walking&&loaded) {
        float translation_y = 0.7;

        glm::mat4 rotatedLeft = glm::rotate(glm::radians(-angle), glm::vec3(1, 0, 0));

        glm::mat4 rotatedRight = glm::rotate(glm::radians(angle), glm::vec3(1, 0, 0));

        m_geometry[LEFT_ARM_I].ctm = rotatedLeft * m_left_arm_ctm;
        m_geometry[RIGHT_ARM_I].ctm = rotatedRight * m_right_arm_ctm;
    }
}

void Player::startAnimation() {
    if(loaded) {
        m_left_ctm = m_geometry[LEFT_LEG_I].ctm;
        m_right_ctm = m_geometry[RIGHT_LEG_I].ctm;
        m_left_arm_ctm = m_geometry[LEFT_ARM_I].ctm;
        m_right_arm_ctm = m_geometry[RIGHT_ARM_I].ctm;
        walking = true;
    } else {
        std::cerr << "Player renderables not loaded" << std::endl;
    }
}

void Player::stopAnimation() {
    if(loaded) {
        walking = false;
        m_geometry[LEFT_LEG_I].ctm = m_left_ctm;
        m_geometry[LEFT_LEG_I].ctm = m_right_ctm;
        m_geometry[LEFT_ARM_I].ctm = m_left_arm_ctm;
        m_geometry[RIGHT_ARM_I].ctm = m_right_arm_ctm;
        m_leg_angle = 0;
    } else {
        std::cerr << "Player renderables not loaded" << std::endl;
    }
}

void Player::insertGeometry(glm::mat4 ctm, PrimitiveType type, int index) {
    RenderObject shape;
    shape.ctm = ctm;
    shape.primitive = {.type = type, .material = m_material};
    m_geometry[index] = shape;
}
