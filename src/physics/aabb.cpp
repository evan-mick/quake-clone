#include "aabb.h"
#include <limits>
#include <algorithm>

AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ):
    points_{minX,minY,minZ,maxX,maxY,maxZ} {

}

AABB::AABB(float points[6]){
    for(int i=0;i<6;i++) {
        points_[i] = points[i];
    }
}

bool AABB::intersects(Ray ray, float& tVal){
    tVal = std::numeric_limits<float>::max();
    for (int i=0;i<3;i++) {
        if(ray.d[i]==0 && (ray.p[i]<points_[i] || ray.p[i]>points_[i+3])) {
            return false;
        } else {
            float tTop = (points_[i+3] - ray.p[i]) / ray.d[i];
            float tBottom = (points_[i] - ray.p[i]) / ray.d[i];
            if(tTop>0 || tBottom > 0) {
                tVal = std::min(tTop,tBottom);
            } else if (tBottom > 0) {
                tVal = tBottom;
            } else if(tTop > 0 ) {
                tVal = tTop;
            }
        }
    }
    return tVal < std::numeric_limits<float>::max();
}

AABB AABB::getTransformedPrimitiveAAB(glm::mat4 ctm){
    glm::vec4 minPoints = ctm * glm::vec4(-0.5,-0.5,-0.5,1.f);;
    glm::vec4 maxPoints = ctm * glm::vec4(0.5,0.5,0.5,1.f);
    float points[6];
    for(int i=0;i<3;i++){
        points[i] = std::min(minPoints[i],maxPoints[i]);
        points[i+3] = std::max(minPoints[i],maxPoints[i]);
    }
    return AABB(points[0],points[1],points[2],points[3],points[4],points[5]);
}

float& AABB::operator[](int index) {
    return points_[index];
}
