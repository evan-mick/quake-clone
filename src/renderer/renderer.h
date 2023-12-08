#ifndef RENDERER_H
#define RENDERER_H
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "scene/sceneparser.h"
#include "camera.h"
#include "movement.h"
#include "rendermodel.h"
#include "objects/player.h"

#include "scene/scenedata.h"
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

class Renderer: public QOpenGLWidget
{
public:
    Renderer(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void clearScreen();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void initializeScene(std::string filepath);
    void handleMovement(float deltaTime);
    void makeFBO();
    void paintGeometry();
    void paintTexture(GLuint texture, bool includePost);
    void insertModel(std::vector<RenderObject>& objects, u_int8_t id, bool isStatic);
    std::vector<RenderObject *> inline getModelObjectsList(std::vector<RenderObject>& shapes) {
        std::vector<RenderObject *> res;
        for(RenderObject& shape : shapes) {
            res.push_back(&shape);
        }
        return res;
    };

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    //RenderData
    SceneData m_metaData;
    GLuint m_shader;
    GLuint m_texture_shader;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    GLuint m_fbo_renderbuffer;
    GLuint m_fbo;
    GLuint m_defaultFBO = 2;
    int m_fbo_width;
    int m_fbo_height;
    GLuint m_fbo_texture;
    Camera m_camera;
    bool parsed_ = false;
    int lightVBO_ = -1;
    Player m_player;
    std::vector<Model> m_models;

    u_int8_t m_model_count = 0;

    // Device Correction Variables
    float m_devicePixelRatio;
    bool parsed;
    std::string prevFile;
    int lightCount;

};
#endif // RENDERER_H
