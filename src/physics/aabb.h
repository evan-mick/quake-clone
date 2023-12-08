#ifndef AABB_H
#define AABB_H
#include <glm/glm.hpp>
#include "scene/scenedata.h"

struct Ray {
    glm::vec4 p;
    glm::vec4 d;
    float t;
};

class AABB
{
public:
    AABB();
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    AABB(float points[6]);
    bool intersects(Ray ray, float& tVal);
    float points_[6];//minx, miny, minz, maxx, maxy, maxz
    static AABB getTransformedPrimitiveAAB(glm::mat4 ctm);
    float& operator[](int index);
};

#endif // AAB_H
