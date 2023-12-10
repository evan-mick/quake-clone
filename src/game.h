#ifndef GAME_H
#define GAME_H

#include "core/ecs.h"
#include "renderer/renderer.h"
#include "physics/physics.h"
#include <GL/glew.h>
#include <glfw/glfw3.h>

// Responsible for compiling all of the separate systems
// ECS, network, renderer, physics, etc.
// This is the class that will register systems n such
class Game
{
public:
    Game();

    void startGame(bool server);
private:
    bool m_running = true;
    bool m_server = false;
    void registerECSComponents(ECS& ecs);
    void registerECSSystems(ECS& ecs, Physics& phys, Renderer& render);
    void registerInputs();

    void setupWindow();

    float m_monitorXScale, m_monitorYScale;

    GLFWwindow* m_window;
};

#endif // GAME_H
