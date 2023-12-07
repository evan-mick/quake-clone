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
 * FLOW:
 * Player input registered before render system
 * Renderable registered as close to last system (has to be after everything that would affect it)
 *
 * ON UPDATE:
 * Clear screen (FUNCTION FOR RENDERER)
 * Input, Physics, movement, etc. all updated
 * Dynamic objects render (FUNCTION FOR RENDERER)
 * - everything with renderable data
 * - "ID" decides what primitives to create and where
 * - POTENTIALLY: two passes, first one to register what primitives will go where, then each primitive is rendered in batches
 *      - if we use this approach, then render system could just add dynamic
 *          objects to primitive sections and they'd be rendered along with static objects so its all bound together.
 * Renderer "render static objects" gets called end of the frame (FUNCTION FOR RENDERER)
 * - Z-buffer should automatically take care of all of this
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

    void clearScreen();
};

#endif // RENDERER_H
