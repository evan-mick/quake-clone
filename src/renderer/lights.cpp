#include "lights.h"

Lights::Lights(std::vector<SceneLightData> lights, const int MAX_LIGHTS) :
    lights_(lights), MAX_LIGHTS_(MAX_LIGHTS)
{
    numLights_ = std::min((int)lights_.size(),MAX_LIGHTS_);
}

void Lights::clearUniformLightData(GLuint shader) {
    std::vector<float> empty4fv;
    empty4fv.assign(numLights_*4,0);
    std::vector<int> empty1iv;
    empty1iv.assign(numLights_,0);
    std::vector<float> empty2fv;
    empty2fv.assign(numLights_*2,0);
    std::vector<float> empty3fv;
    empty3fv.assign(numLights_*3,0);
    glUniform4fv(glGetUniformLocation(shader, "lightPos"), numLights_ * 4, empty4fv.data());
    glUniform1i(glGetUniformLocation(shader, "numLights"), numLights_);
    glUniform4fv(glGetUniformLocation(shader, "lightColor"), numLights_ * 4, empty4fv.data());
    glUniform1iv(glGetUniformLocation(shader, "lightType"), numLights_, empty1iv.data());
    glUniform4fv(glGetUniformLocation(shader, "lightDir"), numLights_ * 4, empty4fv.data());
    glUniform2fv(glGetUniformLocation(shader, "lightAngle"), numLights_ * 2, empty2fv.data());
    glUniform3fv(glGetUniformLocation(shader, "lightAtt"), numLights_ * 3, empty3fv.data());
}

void Lights::setUniformLightData(GLuint shader) {
    glUniform4fv(glGetUniformLocation(shader, "lightPos"), numLights_ * 4, lightPositionData().data());
    glUniform1i(glGetUniformLocation(shader, "numLights"), numLights_);
    glUniform4fv(glGetUniformLocation(shader, "lightColor"), numLights_ * 4, lightColorData().data());
    glUniform1iv(glGetUniformLocation(shader, "lightType"), numLights_, lightTypeData().data());
    glUniform4fv(glGetUniformLocation(shader, "lightDir"), numLights_ * 4, lightDirData().data());
    glUniform2fv(glGetUniformLocation(shader, "lightAngle"), numLights_ * 2, lightAngleData().data());
    glUniform3fv(glGetUniformLocation(shader, "lightAtt"), numLights_ * 3, lightAttData().data());
}

std::vector<float> Lights::lightPositionData() const {
    std::vector<float> lightPosArray(numLights_ * 4);
    for (int i = 0; i < numLights_; ++i) {
        glm::vec4 lightPos = lights_[i].pos;
        lightPosArray[i * 4] = lightPos.x;
        lightPosArray[i * 4 + 1] = lightPos.y;
        lightPosArray[i * 4 + 2] = lightPos.z;
        lightPosArray[i * 4 + 3] = lightPos.w;
    }
    return lightPosArray;
}

std::vector<float> Lights::lightColorData() const {
    std::vector<float> lightColorArray(numLights_ * 4);
    for (int i = 0; i < numLights_; ++i) {
        glm::vec4 lightColor = lights_[i].color;
        lightColorArray[i * 4] = lightColor[0];
        lightColorArray[i * 4 + 1] = lightColor[1];
        lightColorArray[i * 4 + 2] = lightColor[2];
        lightColorArray[i * 4 + 3] = lightColor[3];
    }
    return lightColorArray;
}

std::vector<int> Lights::lightTypeData() const {
    std::vector<int> lightTypeArray(numLights_);
    for (int i = 0; i < numLights_; i++) {
        int typeInt;
        switch(lights_[i].type) {
        case LightType::LIGHT_DIRECTIONAL:
            typeInt = 1;
            break;
        case LightType::LIGHT_POINT:
            typeInt = 0;
            break;
        case LightType::LIGHT_SPOT:
            typeInt = 2;
            break;
        }
        lightTypeArray[i] = typeInt;
    }
    return lightTypeArray;
}

std::vector<float> Lights::lightDirData() const {
    std::vector<float> lightDirArray(numLights_ * 4);
    for (int i = 0; i < numLights_; ++i) {
        glm::vec4 lightDir = lights_[i].dir;
        lightDirArray[i * 4] = lightDir[0];
        lightDirArray[i * 4 + 1] = lightDir[1];
        lightDirArray[i * 4 + 2] = lightDir[2];
        lightDirArray[i * 4 + 3] = lightDir[3];
    }
    return lightDirArray;
}

std::vector<float> Lights::lightAngleData() const {
    std::vector<float> lightAngleArray(numLights_ * 2);
    for (int i = 0; i < numLights_; ++i) {
        lightAngleArray[i * 2] = lights_[i].angle;
        lightAngleArray[i * 2 + 1] = lights_[i].penumbra;
    }
    return lightAngleArray;
}

std::vector<float> Lights::lightAttData() const {
    std::vector<float> lightAttArray(numLights_ * 3);
    for (int i = 0; i < numLights_; ++i) {
        glm::vec3 lightAtt = lights_[i].function;
        lightAttArray[i * 3] = lightAtt[0];
        lightAttArray[i * 3 + 1] = lightAtt[1];
        lightAttArray[i * 3 + 2] = lightAtt[2];
    }
    return lightAttArray;
}
