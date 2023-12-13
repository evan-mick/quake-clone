#ifndef LEVEL_H
#define LEVEL_H
#include "scenedata.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <random>
#define PRIM_COUNT 7


class Level
{
public:
    Level(float x, float y, float z);
    void generateLevel();
    std::vector<Model> getLevelModels();
    glm::vec3 getRandomSpawnpointPos();

private:

//    RenderObject *m_objs;
    std::array<RenderObject,PRIM_COUNT> m_geometry;
    std::vector<Model> m_models;
    void insertSimpleModel(glm::mat4 ctm, PrimitiveType type, int index, bool visible = true);
    void makeSpawnpoints(int count);
    void makeFloor();
    void makeWalls();
    void makeObstacles();
    float size_z;
    float size_x;
    float height;
    std::vector<glm::vec3> m_spawnpoints;
    SceneMaterial m_mat;
};

#endif // LEVEL_H
