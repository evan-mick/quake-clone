//#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <iostream>
#include <QSettings>
#include <iostream>

#include "core/ecs.h"
#include "game_types.h"
#include <random>



int main(int argc, char *argv[]) {
    //    QApplication a(argc, argv);

    //    QCoreApplication::setApplicationName("Nifty Quake Clone);
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

//    int flag = FL_PHYSICS | FL_TRANSFORM | FL_TEST;
    int test_ent = ecs.createEntity({FLN_TEST, FLN_TRANSFORM, FLN_TESTKILL});
//    int test_ent2 = ecs.createEntityWithBitFlags((1 << FLN_TEST) | (1 << FLN_TRANSFORM) | (1 << FLN_TESTKILL));

    Test* tst = static_cast<Test*>(ecs.getComponentData(test_ent, FLN_TEST));
    tst->timer = 1.f;
    tst->ticks = 10;


    system_t test = [](ECS* e, int ent, float delta) {

        Test* test = static_cast<Test*>(e->getComponentData(ent, FLN_TEST));


        test->timer -= delta;


        if (test->timer < 0) {

            std::cout << "HI, I am " << ent << " " << std::to_string(delta) << " " << std::to_string(test->ticks) << std::endl;
            test->ticks--;
            test->timer = 1.0f;

            if (test->ticks < 0) {
                e->queueDestroyEntity(ent);
                return;
            }
        }

    };
    ecs.registerSystem(test, {FLN_TEST});

    ecs.registerSystem([](ECS* e, int ent, float delta) {
        std::cout << std::to_string(e->getEntityBitMask(ent)) << " physics" << std::endl;

        Physics* phys = static_cast<Physics*>(e->getComponentData(ent, FLN_PHYSICS));
        if (phys == nullptr) {
            std::cout << "Test kill err! " << ent << " " << std::to_string(delta) << std::endl;
            e->queueDestroyEntity(ent);
            return;
        }

        if (std::rand() % 3 == 0) {
            e->queueDestroyEntity(ent);
            std::cout << "Test kill activated! " << ent << " " << std::to_string(delta) << std::endl;
        } else {
            std::cout << "Test kill failed! " << ent << " " << std::to_string(delta) << std::endl;
        }
    }, {FLN_TESTKILL, FLN_PHYSICS});




    while (true) {
        ecs.update();
    }

    std::cout << sizeof(glm::vec3) << std::endl;
    //    int return_val = a.exec();
    //    w.finish();
    //    return return_val;
}


