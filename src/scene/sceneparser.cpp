#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, SceneData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    renderData.lights.clear();
    renderData.shapes.clear();

    SceneNode* rootNode = fileReader.getRootNode();
    getRenderShapes(rootNode,
                    &renderData.shapes,
                    &renderData.lights,
                    glm::mat4(1.f));
    return true;
}


bool SceneParser::parse(std::string filepath) {
    return parse(filepath, m_sceneData);
}

void SceneParser::getRenderShapes(SceneNode* node,
                                  std::vector<RenderObject>* shapes,
                                  std::vector<SceneLightData>* lights,
                                  glm::mat4 ctm) {
    for (SceneTransformation* t : node->transformations) {
        switch(t->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = glm::translate(ctm,t->translate);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = glm::rotate(ctm,t->angle,t->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm = glm::scale(ctm,t->scale);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = ctm * t->matrix;
            break;
        }
    }
    for(SceneLight* l : node->lights) {
        SceneLightData lightData;
        lightData.id = l->id;
        lightData.type = l->type;
        lightData.color = l->color;
        lightData.function = l->function;
        if(lightData.type!=LightType::LIGHT_DIRECTIONAL) {
            lightData.pos = ctm * glm::vec4(0.f,0.f,0.f,1.f);
        }
        if(lightData.type!=LightType::LIGHT_POINT) {
            lightData.dir = ctm * l->dir;
        }
        if (lightData.type==LightType::LIGHT_SPOT) {
            lightData.penumbra = l->penumbra;
            lightData.angle = l->angle;
        }
        lightData.width = l->width;
        lightData.height = l->height;
        lights->push_back(lightData);
    }

    for(ScenePrimitive* p : node->primitives) {
        RenderObject s = {*p,ctm};
        shapes->push_back(s);
    }

    for(SceneNode* child : node->children) {
        getRenderShapes(child,shapes,lights,ctm);
    }
}
