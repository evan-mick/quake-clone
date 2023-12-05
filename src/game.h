#ifndef GAME_H
#define GAME_H

#include "core/ecs.h"

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
};

#endif // GAME_H
