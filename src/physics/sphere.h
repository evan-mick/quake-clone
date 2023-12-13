#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <algorithm>

class Sphere {
public:
    glm::vec3 center;
    float radius;

    // initialize sphere with a center and radius
    Sphere(const glm::vec3& center, float radius);

    // check if sphere intersects with another sphere
    bool intersects(const Sphere& other) const;

    // get transformed version of this sphere
    Sphere getTransformedSphere(const glm::mat4& transform) const;
};

#endif // SPHERE_H
