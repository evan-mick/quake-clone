#pragma once

#include "scenedata.h"
#include <vector>
#include <string>



class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(std::string filepath, SceneData &renderData);
    static void getRenderShapes(SceneNode* node,
                                std::vector<RenderObject>* shapes,
                                std::vector<SceneLightData>* lights,
                                glm::mat4 ctm);
    static bool parse(std::string filepath);

    static inline SceneData& getSceneData() {
        return m_sceneData;
    }

    static inline bool hasParsed() {
        return m_hasParsed;
    }

private:
    static inline SceneData m_sceneData {};
    static inline bool m_hasParsed = false;
};
