#pragma once

#include "scenedata.h"
#include <vector>
#include <string>
#include <GL/glew.h>


class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(std::string filepath, RenderData &renderData);
    static void getRenderShapes(SceneNode* node,
                                std::vector<RenderObject>* shapes,
                                std::vector<SceneLightData>* lights,
                                glm::mat4 ctm);
};
