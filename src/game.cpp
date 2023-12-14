


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
        std::cout << "Network setup attempt complete" << std::endl;
    } else {
        std::cout << "No Networking" << std::endl;
    }

    if (!server)
        setupWindow();

    // Parse setup
    SceneParser SCENEparser = SceneParser();
    //    SCENEparser.parse("../../resources/scenes/phong_total.json");
    SCENEparser.parse("resources/scenes/main.json");
    phys.setStaticObs(&SceneParser::getSceneData());
    SceneParser::getSceneData().cameraData.heightAngle = FOV;

    // Putting this here for now, needs to move, but should def not be in renderer
    /*auto m_level(Level(50.f,5.f,50.f));
    auto data = &SceneParser::getSceneData();
    m_level.generateLevel();
    for(Model& mod : m_level.getLevelModels()) {
        for(RenderObject obj : mod.objects) {
            data->shapes.push_back(obj);
        }
    }*/

    Camera cam = Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, SceneParser::getSceneData().cameraData);

    Renderer render = Renderer(&cam, !m_server);

    if (!server)
        render.setRatio(m_monitorXScale, m_monitorYScale);
    //        render.resizeGL(DSCREEN_WIDTH,DSCREEN_HEIGHT);

    registerECSComponents(ecs);
    registerECSSystems(ecs, phys, render);
    registerCollisionResponses(phys);

    if (!m_server)
        registerInputs();


    entity_t ent;
    if (!net)
        ent = createPlayer(&ecs, glm::vec3(0, 50.f, 0));
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

        if (net)
            net->deserializeAllDataIntoECS();

        if (!m_server) {
            InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);
            if (in == nullptr) {

                std::cout << "null input 1" << std::endl;
            }

            if (in) {
                in->dat = Input::getHeld();

                double xpos, ypos;
                glfwGetCursorPos(m_window, &xpos, &ypos);
                in->x_look -= (xpos - last_x_look) * 1/100.f;
                in->y_look -= (ypos - last_y_look) * 1/100.f;

                const float barrier = glm::radians(89.0f);

                if (in->y_look > barrier) { in->y_look = barrier; }
                if (in->y_look < -barrier) { in->y_look = -barrier; }

                last_x_look = xpos;
                last_y_look = ypos;




//                in->y_look = std::clamp(in->y_look, 0.2f, 3.0f);
            }
            render.startDraw();
        }
        // Main simulation logic

        phys.startFrame();
        ecs.update();

        if (!m_server) {
            InputData* in = getComponentData<InputData>(&ecs, ent, FLN_INPUT);
            if (in == nullptr) {
                std::cout << "null input 2" << std::endl;
                //continue;
            }
             render.startDraw();

            render.drawDynamicAndStaticObs();
            render.drawScreen();
            if (ecs.entityHasComponent(ent, FLN_TRANSFORM)) {
                cam.updateFromEnt(&ecs, ent);
                cam.setRotation(in->x_look, in->y_look);
            }

            // std::cout << "3" << std::endl;
            // Swap front and back buffers
            glfwSwapBuffers(m_window);

            // Poll for and process events
            glfwPollEvents();

        }


        if (net) {
            net->broadcastOnTick(ecs.getRecentDelta());
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

    ecs.registerComponent(FLN_RENDER, sizeof(Renderable));
    ecs.registerComponent(FLN_DESTROYTIME, sizeof(DestroyData));

    ecs.registerComponent(FLN_TEST, sizeof(Test));
    ecs.registerComponent(FLN_TYPE, sizeof(TypeData));

    ecs.registerComponent(FLN_PROJECTILE, sizeof(Projectile));
    ecs.registerComponent(FLN_HEALTH, sizeof(Health));
    ecs.registerComponent(FLN_PLAYERINFO, sizeof(PlayerInfo));
}

void Game::registerInputs() {
    Input::registerHeld(GLFW_KEY_W, IN_FORWARD);
    Input::registerHeld(GLFW_KEY_S, IN_BACK);
    Input::registerHeld(GLFW_KEY_A, IN_LEFT);
    Input::registerHeld(GLFW_KEY_D, IN_RIGHT);
    Input::registerHeld(GLFW_KEY_Z, IN_SHOOT);
    Input::registerHeld(GLFW_KEY_SPACE, IN_JUMP);
}

void Game::registerCollisionResponses(Physics& phys) {


    phys.registerType(ET_PROJ, [](ECS* e, entity_t my_ent, entity_t other_ent, bool world) -> glm::vec3 {

        std::cout << "proj col" << std::endl;
//        if (world) {

        ent_type_t type = getType(e, other_ent);

        if ((world || (getComponentData<Projectile>(e, my_ent, FLN_PROJECTILE)->shot_from != other_ent && type == ET_PLAYER))){
            if (e->hasAuthority(my_ent)) {
                entity_t explode = createExplosion(e, getTransform(e, my_ent)->pos - glm::vec3(0, 1, 0));
                entity_t other_shot = getComponentData<Projectile>(e, my_ent, FLN_PROJECTILE)->shot_from;
                getComponentData<Projectile>(e, explode, FLN_PROJECTILE)->shot_from = other_shot;
            }
            e->queueDestroyEntity(my_ent);
        }

        return glm::vec3(0, 0, 0);
    });

    phys.registerType(ET_EXPLOSION, [](ECS* e, entity_t my_ent, entity_t other_ent, bool world) -> glm::vec3 {

        DestroyData* destroyDat = getComponentData<DestroyData>(e, my_ent, FLN_DESTROYTIME);
        if (!world /*&& 1.f - destroyDat->timer < TICK_RATE * 5.f*/ && getType(e, other_ent) == ET_PLAYER) {
            std::cout << "explosion col" << std::endl;
            PhysicsData* dat = getPhys(e, other_ent);
            PlayerInfo* info = getComponentData<PlayerInfo>(e, other_ent, FLN_PLAYERINFO);

            Transform* other_trans = getTransform(e, other_ent);
            Transform* trans = getTransform(e, my_ent);

            if (!e->hasAuthority(other_ent))
                return glm::vec3(0, 0, 0);


            if (dat && info) {
                info->no_control_time = .5f;
//                std::cout << "explosion col 2" << std::endl;
                glm::vec3 dir = glm::normalize(((other_trans->pos + glm::vec3(0, 4.f, 0)) - (trans->pos)));

                if (info->invul > 0.f)
                    return glm::vec3(0, 0, 0);

//                dat->vel.y += 5.f;

                dat->vel += dir * 40.f;

                dat->grounded = false;

                info->invul = .2f;

                if (getComponentData<Projectile>(e, my_ent, FLN_PROJECTILE)->shot_from != other_ent) {

                    getComponentData<Health>(e, other_ent, FLN_HEALTH)->amt -= PROJ_DMG;
                    std::cout << "HEALTH " << other_ent << " " << getComponentData<Health>(e, other_ent, FLN_HEALTH)->amt << std::endl;

                    if (getComponentData<Health>(e, other_ent, FLN_HEALTH)->amt <= 0) {
                        respawnPlayer(e, other_ent);
                    }
                }
            }

        }

        return glm::vec3(0, 0, 0);
    });
}



void Game::registerECSSystems(ECS& ecs, Physics& phys, Renderer& renderer) {
    ecs.registerSystemWithBitFlags([&phys](ECS* e, entity_t ent, float delta) { phys.tryRunStep(e, ent, delta); }, phys.getRequiredFlags());

    // Rendering system
    if (!m_server)
        ecs.registerSystem([&renderer](ECS* e, entity_t ent, float delta) {
//            renderer.drawDynamicOb(e, ent, delta);

            if (e->hasAuthority(ent) && getType(e, ent) == ET_PLAYER)
                return;

            renderer.queueDynamicModel(e,ent,delta);
        } , {FLN_TRANSFORM, FLN_RENDER});

    // Player Fire Rocket input
    if (!m_server)
        ecs.registerSystem([](ECS* e, entity_t ent, float delta) {

            // What if input down but rocket never made?
            if (!e->hasAuthority(ent))
                return;

            float cool = (getComponentData<PlayerInfo>(e, ent, FLN_PLAYERINFO)->shotCooldown -= delta);

            Transform* trans = getTransform(e, ent);
            InputData* in = getComponentData<InputData>(e, ent, FLN_INPUT);
            if (cool <= 0.f && Input::isHeld(in->dat, IN_SHOOT) && !Input::isHeld(in->last_dat, IN_SHOOT)) {
                glm::vec3 look_;
                look_.x = -sin(in->x_look) * cos(in->y_look);
                look_.y = sin(in->y_look);
                look_.z = -cos(in->x_look) * cos(in->y_look);

                int proj = createProjectile(e, trans->pos, look_);
                Projectile* projdat = getComponentData<Projectile>(e, proj, FLN_PROJECTILE);
                projdat->shot_from = ent;

                getComponentData<PlayerInfo>(e, ent, FLN_PLAYERINFO)->shotCooldown = .8f;
            }
        }

    , {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM, FLN_INPUT, FLN_PLAYERINFO});

    ecs.registerSystem([](ECS* e, entity_t ent, float delta) {
        DestroyData* destroyDat = getComponentData<DestroyData>(e, ent, FLN_DESTROYTIME);
        destroyDat->timer -= delta;

        if (destroyDat->timer <= 0.f) {
            e->queueDestroyEntity(ent);
        }
    }, { FLN_DESTROYTIME });

    // Player Movement Input
    ecs.registerSystem([&renderer](ECS* e, entity_t ent, float delta) {

        // What if some desync happens? then we'd want this to run but it won't cause no authority
        // Can't take away though cause then jitter (?)
        if (!e->hasAuthority(ent))
            return;

        //        std::cout << "thing" << std::endl;
        PhysicsData* phys = getPhys(e, ent);
        Transform* trans = getTransform(e, ent);
        Test* ts = getComponentData<Test>(e, ent, FLN_TEST);
        InputData* in = getComponentData<InputData>(e, ent, FLN_INPUT);
        PlayerInfo* info = getComponentData<PlayerInfo>(e, ent, FLN_PLAYERINFO);
        //        std::cout << "test " << ent << std::endl;

        renderer.player_health = ((getComponentData<Health>(e, ent, FLN_HEALTH)->amt)/MAX_HEALTH);

        phys->accel = glm::vec3(0, -.98f, 0);

        if (info->invul > 0.f)
            info->invul -= delta;

        trans->rot.y = in->x_look/* - glm::radians(53.f)*/;

        // Needed the 37 degree offset?? no clue why, it would be consistently slightly off whenever I moved it
        glm::mat4 forwardMatrix = glm::rotate(glm::mat4(1.0f), in->x_look + glm::radians(90.f), glm::vec3(0.0f, 1.0f, 0.0f));
        forwardMatrix = glm::translate(forwardMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
        glm::vec3 forwardDirection = forwardMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::mat4 sideMatrix = glm::rotate(glm::mat4(1.0f), in->x_look /*+ glm::radians(127.f)*/, glm::vec3(0.0f, 1.0f, 0.0f));
        sideMatrix = glm::translate(sideMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
        glm::vec3 sideDirection = sideMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 vel = glm::vec3(0, 0, 0);
        if (Input::isHeld(in->dat, IN_FORWARD)) {
            vel += forwardDirection;
        } else if (Input::isHeld(in->dat, IN_BACK)) {
            vel += -forwardDirection;
        }

        if (Input::isHeld(in->dat, IN_RIGHT)) {
            vel += sideDirection;
        } else if (Input::isHeld(in->dat, IN_LEFT)) {
            vel += -sideDirection;
        }

        // TO BE IMPROVED, the actual quake accel code, doesn't really work rn

        glm::vec3 norm_vel = glm::normalize(vel);

//        glm::vec3 norm_vel = glm::normalize(vel) * 10.f;

        if (info->no_control_time <= 0.f) {
            if (vel != glm::vec3(0, 0, 0) && phys->grounded) {
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
            if (Input::isHeld(in->dat, IN_JUMP) && phys->grounded) {
                phys->vel.y = 15.f;
                phys->vel.x *= 1.15f;
                phys->vel.z *= 1.15f;
                phys->grounded = false;
            }
            if (phys->grounded) {
                phys->vel.x *= .99f * delta;
                phys->vel.z *= .99f * delta;
            }

            if (trans->pos.y < -100.f) {
                respawnPlayer(e, ent);
            }
        } else {
            info->no_control_time -= delta;
        }

//        else
//            phys->vel = glm::vec3(0, phys->vel.y, 0);

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




        in->last_dat = in->dat;
        ts->timer += delta;

        //        trans->pos = glm::vec3(2.f * glm::acos(ts->timer), trans->pos.y, trans->pos.z);
    }, {FLN_TEST, FLN_PHYSICS, FLN_TRANSFORM, FLN_INPUT, FLN_HEALTH});

}
