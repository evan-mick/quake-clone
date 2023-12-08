#ifndef LIGHTS_H
#define LIGHTS_H
#include "scene/scenedata.h"

class Lights
{
public:
    Lights(std::vector<SceneLightData> lights, const int MAX_LIGHTS);
    void setUniformLightData(GLuint shader);
    void clearUniformLightData(GLuint shader);
private:
    std::vector<float> lightPositionData() const;
    std::vector<float> lightColorData() const;
    std::vector<float> lightDirData() const;
    std::vector<float> lightAngleData() const;
    std::vector<float> lightAttData() const;
    std::vector<int> lightTypeData() const;
    std::vector<SceneLightData> lights_;
    const int MAX_LIGHTS_;
    int numLights_;
};

#endif
