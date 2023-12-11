


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
#include <algorithm>
#include "scene/sceneparser.h"
#include "glm/gtx/rotate_vector.hpp"

//#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glfw/glfw3.h>

#include "game_create_helpers.h"


Game::Game()
{

}

void Game::startGame(bool server, const char* ip) {

    std::cout << "Starting Game" << std::endl;
    m_server = server;

    ECS ecs = ECS();
    Physics phys = Physics(TICK_RATE);
    std::cout << "Phys ECS" << std::endl;

    std::unique_ptr<Network> net;
    // disabling network for now
    if (std::string(ip) != "" || server) {
        std::cout << "Network setup: " << (server ? "server" : "client connecting to " + std::string(ip)) << std::endl;
        net = std::make_unique<Network>(server, &ecs, ip);

        // set authority on entity create
        entbroadcast_t bound = [&net](entity_t ent) { net->setAuthority(ent); };
        ecs.addBroadcast(bound);
        std::cout << "Network setup attempt complete" << std::endl;
    } else {
        std::cout << "No Networking" << std::endl;
    }

    if (!server)
        setupWindow();

    // Parse setup
    SceneParser SCENEparser = SceneParser();
    //    SCENEparser.parse("../../resources/scenes/phong_total.json");
    SCENEparser.parse("../../resources/scenes/empty.json");
    phys.setStaticObs(&SceneParser::getSceneData());
    SceneParser::getSceneData().cameraData.heightAngle = FOV;

    // Putting this here for now, needs to move, but should def not be in renderer
    auto m_level(Level(50.f,5.f,50.f));
    auto data = &SceneParser::getSceneData();
    m_level.generateLevel();
    for(Model& mod : m_level.getLevelModels()) {
        for(RenderObject obj : mod.objects) {
            data->shapes.push_back(obj);
        }
    }

    Camera cam = Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, SceneParser::getSceneData().cameraData);

    Renderer render = Renderer(&cam, !m_server);

    if (!server)
        render.setRatio(m_monitorXScale, m_monitorYScale);
    //        render.resizeGL(DSCREEN_WIDTH,DSCREEN_HEIGHT);

    registerECSComponents(ecs);
    registerECSSystems(ecs, phys, render);

    if (!m_server)
        registerInputs();


    entity_t ent;
    if (!net)
        ent = createPlayer(&ecs, glm::vec3(0, 10.f, 0));
    else if (!m_server) {
        ent = net->getMyPlayerEntityID();
        std::cout<< "player ent: " << std::to_string(ent) << std::endl;
    }

    //    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    //        Renderer::default_render->resizeGL(width, height);
    //    });

    float last_x_look = 0;
    float last_y_look = 0;

    while (m_running) {

        if (net) {
            net->deserializeAllDataIntoECS(&ecs);

        //        Input::checkKeys(window);
        //        if (Input::getHeld())
        //            std::cout << "held " << Input::getHeld() << std::endl;
        //InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);

        if (!m_server) {
            InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);
            in->dat = Input::getHeld();

            double xpos, ypos;
            glfwGetCursorPos(m_window, &xpos, &ypos);
            in->x_look -= (xpos - last_x_look) * 1/100.f;
            in->y_look += (ypos - last_y_look) * 1/100.f;
            last_x_look = xpos;
            last_y_look = ypos;

            in->y_look = std::clamp(in->y_look, 0.2f, 3.0f);
            //         getComponentData<InputData>(&ecs, ent, FLN_INPUT)-> = Input::getHeld();

            //        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        }
        // Main simulation logic

        phys.startFrame();
         ecs.update();

        if (!m_server) {
            InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);
            cam.updateFromEnt(&ecs, ent);
            cam.setRotation(in->x_look, in->y_look);

            render.startDraw();

            render.drawStaticObs();
            render.drawDynamicObs();

            render.drawScreen();
            // std::cout << "3" << std::endl;
            // Swap front and back buffers
            glfwSwapBuffers(m_window);
            // std::cout << "4" << std::endl;
            // Poll for and process events
            glfwPollEvents();
            // std::cout << "second input data block end" << std::endl;
        }


        if (net)
            net->broadcastOnTick(ecs.getRecentDelta());
            // std::cout << "tick broadcasted (game)" << std::endl;
        }
    }
    glfwTerminate();

}


void Game::setupWindow() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Set GLFW to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(DSCREEN_WIDTH, DSCREEN_HEIGHT, "Quake Clone", nullptr, nullptr);
    m_window = window;

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

    //    float xscale, yscale;
    int realWidth,realHeight;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetWindowContentScale(window, &m_monitorXScale,&m_monitorYScale);
    std::cout <<" window is" << realWidth <<","<<realHeight << std::endl;


    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }



    glfwSetKeyCallback(window, Input::key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


}


void Game::registerECSComponents(ECS& ecs) {
    ecs.registerComponent(FLN_PHYSICS, sizeof(PhysicsData));
    ecs.registerComponent(FLN_COLLISION, sizeof(CollisionData));
    ecs.registerComponent(FLN_TRANSFORM, sizeof(Transform));
    ecs.registerComponent(FLN_INPUT, sizeof(InputData));

    //    if (!m_server)
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
    ecs.registerSystemWithBitFlags([&phys](ECS* e, entity_t ent, float delta) { phys.tryRunStep(e, ent, delta); }, phys.getRequiredFlags());

    if (!m_server)
        ecs.registerSystem([&renderer](ECS* e, entity_t ent, float delta) {
//            renderer.drawDynamicOb(e, ent, delta);
            renderer.queueDynamicModel(e,ent,delta);
        } , {FLN_TRANSFORM, FLN_RENDER});

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

        phys->accel = glm::vec3(0, -.98f, 0);


        trans->rot.y = in->x_look - glm::radians(53.f);


        // Needed the 37 degree offset?? no clue why, it would be consistently slightly off whenever I moved it
        glm::mat4 forwardMatrix = glm::rotate(glm::mat4(1.0f), in->x_look + glm::radians(37.f), glm::vec3(0.0f, 1.0f, 0.0f));
        forwardMatrix = glm::translate(forwardMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
        glm::vec3 forwardDirection = forwardMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::mat4 sideMatrix = glm::rotate(glm::mat4(1.0f), in->x_look + glm::radians(127.f), glm::vec3(0.0f, 1.0f, 0.0f));
        sideMatrix = glm::translate(sideMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
        glm::vec3 sideDirection = sideMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 vel = glm::vec3(0, 0, 0);
        if (Input::isHeld(in->dat, IN_FORWARD)) {
            vel += -forwardDirection;
        } else if (Input::isHeld(in->dat, IN_BACK)) {
            vel += forwardDirection;
        }

        if (Input::isHeld(in->dat, IN_RIGHT)) {
            vel += sideDirection;
        } else if (Input::isHeld(in->dat, IN_LEFT)) {
            vel += -sideDirection;
        }

        // TO BE IMPROVED, the actual quake accel code, doesn't really work rn

        glm::vec3 norm_vel = glm::normalize(vel);

//        glm::vec3 norm_vel = glm::normalize(vel) * 10.f;

        if (vel != glm::vec3(0, 0, 0)) {
//            phys->vel.x = norm_vel.x;
//            phys->vel.z = norm_vel.z;

            float current = glm::dot(phys->vel, norm_vel);
            float wishspeed = 10.f;
            float addspeed = wishspeed - current;
            float accelspeed = 9.f * wishspeed;
            if (addspeed > 0) {
                if (accelspeed > addspeed) {
                    accelspeed = addspeed;
                }
                phys->accel.x = norm_vel.x * accelspeed;
                phys->accel.z = norm_vel.z * accelspeed;
            }

        }
//        else
//            phys->vel = glm::vec3(0, phys->vel.y, 0);

        //        int			i;
        //        float		addspeed, accelspeed, currentspeed;


        /*currentspeed = DotProduct (pm->ps->velocity, wishdir);
        addspeed = wishspeed - currentspeed;
        if (addspeed <= 0) {
            return;
        }
        accelspeed = accel*pml.frametime*wishspeed;
        if (accelspeed > addspeed) {
            accelspeed = addspeed;
        }

        for (i=0 ; i<3 ; i++) {
            pm->ps->velocity[i] += accelspeed*wishdir[i];
        }*/






        if (Input::isHeld(in->dat, IN_JUMP) && phys->grounded) {
            phys->vel = glm::vec3(0, 15.f, 0);
        }

        if (Input::isHeld(in->dat, IN_SHOOT) && !Input::isHeld(in->last_dat, IN_SHOOT)) {
            int proj = createProjectile(e, trans->pos, glm::vec2(in->x_look, in->y_look));
        }

        in->last_dat = in->dat;
        ts->timer += delta;

        //        trans->pos = glm::vec3(2.f * glm::acos(ts->timer), trans->pos.y, trans->pos.z);
    }, {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM, FLN_INPUT});

}
