//

#include "game.h"
#include "core/ecs.h"
#include "network/network.h"
#include "renderer/renderer.h"
#include "game_types.h"
#include "physics/physics.h"
//#include <QApplication>
//#include <QScreen>
#include <iostream>
//#include <QSettings>

#include "scene/sceneparser.h"

//#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glfw/glfw3.h>


Game::Game()
{

}

void Game::startGame(bool server) {

    std::cout << "Starting Game" << std::endl;
    m_server = server;

    ECS ecs = ECS();
    Physics phys = Physics(TICK_RATE);
    std::cout << "Phys ECS" << std::endl;

    // disabling network for now
    // Network net = Network(server, &ecs);
//    std::cout << "Net" << std::endl;

    registerECSComponents(ecs);



    std::cout << "ECS Setup Complete" << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
//        perror("couldn't create window: ");
//        fprintf(stderr, "Failed to create window\n");
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set GLFW to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on macOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(DSCREEN_WIDTH, DSCREEN_HEIGHT, "Quake Clone", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        const char* description;
        glfwGetError(&description);

        std::cout << description << std::endl;

        glfwTerminate();
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
//    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    float xscale, yscale;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    SceneParser SCENEparser = SceneParser();
//    SCENEparser.parse("../../resources/scenes/phong_total.json");
    SCENEparser.parse("../../resources/scenes/empty.json");


    Renderer render = Renderer();
    Renderer::default_render->setRatio(xscale, yscale);

    registerECSSystems(ecs, phys, render);

    entity_t ent = ecs.createEntity({ FLN_TRANSFORM, FLN_PHYSICS, FLN_TEST, FLN_RENDER });
    Renderable* rend = static_cast<Renderable*>(ecs.getComponentData(ent, FLN_RENDER));
//    rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
    rend->model_id = 5;

    Transform* trans = static_cast<Transform*>(ecs.getComponentData(ent, FLN_TRANSFORM));
    trans->pos = glm::vec3(0, 0, 0);
    trans->scale = glm::vec3(1, 1, 1);


//    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
//        Renderer::default_render->resizeGL(width, height);
//    });

    while (m_running) {

//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render.startDraw();

        ecs.update();

        render.drawStaticObs();
        render.drawScreen();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();


    }
    glfwTerminate();

}


void Game::registerECSComponents(ECS& ecs) {
    ecs.registerComponent(FLN_PHYSICS, sizeof(PhysicsData));
    ecs.registerComponent(FLN_COLLISION, sizeof(CollisionData));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_INPUT, sizeof(InputData));
    ecs.registerComponent(FLN_RENDER, sizeof(Renderable));
    ecs.registerComponent(FLN_TEST, sizeof(Test));
}


void Game::registerECSSystems(ECS& ecs, Physics& phys, Renderer& renderer) {
    ecs.registerSystemWithBitFlags(Physics::tryRunStep, phys.getRequiredFlags());
    ecs.registerSystem(Renderer::drawDynamicOb, {FLN_TRANSFORM, FLN_RENDER});

    ecs.registerSystem([](ECS* e, entity_t ent, float delta) {

//        std::cout << "thing" << std::endl;
        PhysicsData* phys = static_cast<PhysicsData*>(e->getComponentData(ent, FLN_PHYSICS));
        Transform* trans = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
        Test* ts = static_cast<Test*>(e->getComponentData(ent, FLN_TEST));

        ts->timer += delta;

        trans->pos = glm::vec3(2.f * glm::cos(ts->timer), trans->pos.y, trans->pos.z);






    }, {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM});

}
