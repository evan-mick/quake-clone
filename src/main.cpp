//#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <iostream>
#include <QSettings>
#include <iostream>

#include "ecs.h"
#include "game_types.h"
#include <random>



int main(int argc, char *argv[]) {
    //    QApplication a(argc, argv);

    //    QCoreApplication::setApplicationName("Projects 5 & 6: Lights, Camera & Action!");
    //    QCoreApplication::setOrganizationName("CS 1230");
    //    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    //    QSurfaceFormat fmt;
    //    fmt.setVersion(4, 1);
    //    fmt.setProfile(QSurfaceFormat::CoreProfile);
    //    QSurfaceFormat::setDefaultFormat(fmt);

    //    MainWindow w;
    //    w.initialize();
    //    w.resize(800, 600);
    //    w.show();

    ECS ecs = ECS();

    ecs.registerComponent(FLN_PHYSICS, sizeof(Physics));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_TEST, sizeof(Test));

    int flag = FL_PHYSICS | FL_TRANSFORM | FL_TEST | FL_TESTKILL;
    int test_ent = ecs.createEntity(flag);
    int test_ent2 = ecs.createEntity( FL_TRANSFORM | FL_TESTKILL);

    system_t test = [](ECS* e, int ent, float delta) {
        std::cout << "HI, I am " << ent << " " << "delta" << std::endl;
    };

    ecs.registerSystem([](ECS* e, int ent, float delta) {

        Physics* phys = static_cast<Physics*>(e->getComponentData(ent, FLN_PHYSICS));
        if (phys == nullptr) {
            std::cout << "Test kill err! " << ent << std::endl;
            e->queueDestroyEntity(ent);
            return;
        }

        if (std::rand() % 3 == 0) {
            e->queueDestroyEntity(ent);
            std::cout << "Test kill activated! " << ent << std::endl;
        } else {
            std::cout << "Test kill failed! " << ent << std::endl;
        }
    }, FL_TESTKILL);


    ecs.registerSystem(test, FL_TEST);

    while (true) {
        ecs.update();
    }

    std::cout << sizeof(glm::vec3) << std::endl;
    //    int return_val = a.exec();
    //    w.finish();
    //    return return_val;
}


