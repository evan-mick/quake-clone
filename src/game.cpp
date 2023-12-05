#include "game.h"
#include "core/ecs.h"
#include "network/Network.h"
#include "renderer/renderer.h"
#include "game_types.h"

Game::Game()
{

}

void Game::startGame(bool server) {
    m_server = server;

    ECS ecs = ECS();
    Network net = Network(server, &ecs);

//    if (!server)
//        net.connect();


    while (m_running) {
        //
        ecs.update();
    }
}


void Game::registerECSComponents(ECS& ecs) {
    ecs.registerComponent(FLN_PHYSICS, sizeof(Physics));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_INPUT, sizeof(InputData));
    ecs.registerComponent(FLN_RENDER, sizeof(Renderable));
    ecs.registerComponent(FLN_TEST, sizeof(Test));
}
