#ifndef RENDERMODEL_H
#define RENDERMODEL_H
#include "scene/scenedata.h"



class RenderModel
{
public:
    RenderModel(std::vector<RenderObject> objects, u_int8_t id);
    void initializeGLGeometry(int param1, int param2);
    void paintModel(GLuint shader);


private:

    std::vector<RenderObject> m_objects;
    u_int8_t m_id;

};

#endif // RENDERMODEL_H
