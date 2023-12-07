#ifndef RENDERER_H
#define RENDERER_H

#include "renderer/scenedata.h"
#include "GL/glew.h"


/* Structure
 * - Shapes folder with primitives
 *
 * Objects with "type" flag can then get model and collision data
 *
 * Other considerations
 * - camera
 *
 *
 */

struct RenderOb {
    uint8_t model_id;
    ScenePrimitive primitive;
    glm::mat4 ctm;
    GLuint vbo;
    GLuint vao;
};


class Renderer
{
public:
    Renderer();
};

#endif // RENDERER_H
