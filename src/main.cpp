//#include "mainwindow.h"


#include "QtGui/qsurfaceformat.h"
#include "core/ecs.h"
#include "game_types.h"
#include <random>
#include "game.h"
#include <iostream>


void test_main();

int main(int argc, char *argv[]) {



    bool server = (argc > 1 && strcmp(argv[1], "s"));
    Game game = Game();
    game.startGame(server);
}



void test_main() {
    ECS ecs = ECS();

    ecs.registerComponent(FLN_PHYSICS, sizeof(PhysicsData));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_TEST, sizeof(Test));
    std::cout << "hi" << std::endl;


//    int flag = FL_PHYSICS | FL_TRANSFORM | FL_TEST;
    entity_t test_ent = ecs.createEntity({FLN_TEST, FLN_TRANSFORM, FLN_TESTKILL});
    std::cout << std::to_string(ecs.getEntityBitMask(test_ent)) << std::endl;
    //    int test_ent2 = ecs.createEntityWithBitFlags((1 << FLN_TEST) | (1 << FLN_TRANSFORM) | (1 << FLN_TESTKILL));

    Test* tst = static_cast<Test*>(ecs.getComponentData(test_ent, FLN_TEST));
    tst->timer = 1.f;
    tst->ticks = 10;

    system_t test = [](ECS* e, entity_t ent, float delta) {

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

    ecs.registerSystem([](ECS* e, entity_t ent, float delta) {
        std::cout << std::to_string(e->getEntityBitMask(ent)) << " physics" << std::endl;

        PhysicsData* phys = static_cast<PhysicsData*>(e->getComponentData(ent, FLN_PHYSICS));
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
}
