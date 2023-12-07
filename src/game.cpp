#include "game.h"
#include "core/ecs.h"
#include "network/Network.h"
#include "renderer/renderer.h"
#include "game_types.h"
#include "physics/physics.h"
#include <QApplication>
#include <QScreen>
#include <iostream>
#include <QSettings>
#include "renderer/mainwindow.h"

Game::Game()
{

}

void Game::startGame(bool server) {
    m_server = server;

    ECS ecs = ECS();
    Physics phys = Physics(TICK_RATE);

    Network net = Network(server, &ecs);

    registerECSComponents(ecs);

    ecs.registerSystemWithBitFlags(Physics::tryRunStep, phys.getRequiredFlags());

//    if (!server)
//        net.connect();
    //TODO: move window generation to its correct space after ECS connection
    int args = 0;
    char* argv;
    QApplication a(args,&argv);

    QCoreApplication::setApplicationName("Nifty Quake Clone");
    QCoreApplication::setOrganizationName("CS 1230");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QSurfaceFormat fmt;
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.initialize();
    w.resize(800, 600);
    w.show();
    a.exec();
    w.finish();


    while (m_running) {
        //
        ecs.update();
    }
}


void Game::registerECSComponents(ECS& ecs) {
    ecs.registerComponent(FLN_PHYSICS, sizeof(PhysicsData));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_INPUT, sizeof(InputData));
    ecs.registerComponent(FLN_RENDER, sizeof(Renderable));
    ecs.registerComponent(FLN_TEST, sizeof(Test));
}
