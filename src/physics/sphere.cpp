#include <glm/glm.hpp>
#include <algorithm>

class Sphere {
public:
    glm::vec3 center;
    float radius;

    Sphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}

    bool intersects(const Sphere& other) const {
        float distance = glm::distance(center, other.center);
        return distance < (radius + other.radius);
    }

    Sphere getTransformedSphere(const glm::mat4& transform) const {
        glm::vec4 transformedCenter = transform * glm::vec4(center, 1.0f);

        float scaleFactor = glm::length(glm::vec3(transform[0])); 
        return Sphere(glm::vec3(transformedCenter), radius * scaleFactor);
    }
};
