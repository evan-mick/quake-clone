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
#include "input.h"

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

    Camera cam = Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, SceneParser::getSceneData().cameraData);

    Renderer render = Renderer(&cam);
    Renderer::default_render->setRatio(xscale, yscale);

    entity_t ent = ecs.createEntity({ FLN_TRANSFORM, FLN_PHYSICS, FLN_TEST, FLN_RENDER, FLN_INPUT });
    Renderable* rend = static_cast<Renderable*>(ecs.getComponentData(ent, FLN_RENDER));
    rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);
//    rend->model_id = 5;
    registerInputs();

    Transform* trans = static_cast<Transform*>(ecs.getComponentData(ent, FLN_TRANSFORM));
    trans->pos = glm::vec3(0.f, 3.f, 0);
    trans->scale = glm::vec3(1, 1, 1);


//    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
//        Renderer::default_render->resizeGL(width, height);
//    });
    glfwSetKeyCallback(window, Input::key_callback);

    registerECSSystems(ecs, phys, render);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float last_x_look = 0;
    float last_y_look = 0;

    while (m_running) {

//        Input::checkKeys(window);
//        if (Input::getHeld())
//            std::cout << "held " << Input::getHeld() << std::endl;
        InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);
        in->dat = Input::getHeld();

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        in->x_look -= (xpos - last_x_look) * 1/100.f;
        in->y_look += (ypos - last_y_look) * 1/100.f;
        last_x_look = xpos;
        last_y_look = ypos;
//         getComponentData<InputData>(&ecs, ent, FLN_INPUT)-> = Input::getHeld();

//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render.startDraw();
        Physics::phys->startFrame();

        ecs.update();
        cam.updateFromEnt(&ecs, ent);
        cam.setRotation(in->x_look, in->y_look);

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

void Game::registerInputs() {
    Input::registerHeld(GLFW_KEY_W, IN_FORWARD);
    Input::registerHeld(GLFW_KEY_S, IN_BACK);
    Input::registerHeld(GLFW_KEY_A, IN_LEFT);
    Input::registerHeld(GLFW_KEY_D, IN_RIGHT);
    Input::registerHeld(GLFW_KEY_Z, IN_SHOOT);
    Input::registerHeld(GLFW_KEY_SPACE, IN_JUMP);
}


void Game::registerECSSystems(ECS& ecs, Physics& phys, Renderer& renderer) {
    ecs.registerSystemWithBitFlags(Physics::tryRunStep, phys.getRequiredFlags());
    ecs.registerSystem(Renderer::drawDynamicOb, {FLN_TRANSFORM, FLN_RENDER});

//    ecs.registerSystem([](ECS* e, entity_t ent, float delta) {

////        std::cout << "thing" << std::endl;
//        PhysicsData* phys = getPhys(e, ent);
//        Transform* trans = getTransform(e, ent);
//        Test* ts = getComponentData<Test>(e, ent, FLN_TEST);

//        ts->timer += delta;

////        trans->pos = glm::vec3(2.f * glm::cos(ts->timer), trans->pos.y, trans->pos.z);
//    }, {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM});

    ecs.registerSystem([](ECS* e, entity_t ent, float delta) {

        //        std::cout << "thing" << std::endl;
        PhysicsData* phys = getPhys(e, ent);
        Transform* trans = getTransform(e, ent);
        Test* ts = getComponentData<Test>(e, ent, FLN_TEST);
        InputData* in = getComponentData<InputData>(e, ent, FLN_INPUT);
//        std::cout << "test " << ent << std::endl;

        if (Input::isHeld(in->dat, IN_FORWARD)) {
//            std::cout << "hi" << std::endl;
            phys->vel = glm::vec3(1, 1, 1);
        }
        if (Input::isHeld(in->dat, IN_SHOOT) && !Input::isHeld(in->last_dat, IN_SHOOT)) {
            int proj = e->createEntity({FLN_TRANSFORM, FLN_PHYSICS, FLN_RENDER});
            getTransform(e, proj)->pos = trans->pos;
            getTransform(e, proj)->scale = glm::vec3(.5f, .5f, .5f);

            Renderable* rend = static_cast<Renderable*>(e->getComponentData(proj, FLN_RENDER));
            rend->model_id = static_cast<uint8_t>(PrimitiveType::PRIMITIVE_SPHERE);

            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), in->x_look, glm::vec3(0.0f, 1.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, in->y_look, glm::vec3(0.0f, 0.0f, 1.0f));
            getPhys(e, proj)->vel = rotationMatrix * glm::vec4(1, 1, 1, 1);

        }


        in->last_dat = in->dat;
        ts->timer += delta;

//        trans->pos = glm::vec3(2.f * glm::cos(ts->timer), trans->pos.y, trans->pos.z);
    }, {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM, FLN_INPUT});

}
