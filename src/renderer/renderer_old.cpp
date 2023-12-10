#include "renderer.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "scene/settings.h"
#include "objects/cube.h"
#include "objects/sphere.h"
#include "objects/cone.h"
#include "objects/cylinder.h"
#include "objects/player.h"
#include "ShaderLoader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "lights.h"
#include "objects/trimesh.h"

#include "game_types.h"


Renderer::Renderer()//(QWidget *parent)
    //: QOpenGLWidget(parent)
{
    m_renderer = this;
    m_prev_mouse_pos = glm::vec2(DSCREEN_WIDTH/2, DSCREEN_HEIGHT/2);//glm::vec2(size().width()/2, size().height()/2);
//    setMouseTracking(true);
//    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Renderer::finish() {
//    killTimer(m_timer);
//    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

//    this->doneCurrent();
}


inline void glErrorCheck(){
    GLenum error = glGetError();
    while(error != GL_NO_ERROR){
        std::cout<<error<<std::endl;
        error = glGetError();
    }
}

void Renderer::initializeGL() {
//    m_devicePixelRatio = this->devicePixelRatio();
    // could be wrong
    m_devicePixelRatio = DSCREEN_WIDTH/DSCREEN_HEIGHT;
    m_camera = Camera();

//    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();


    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // This file path stuff ain't ideal? for whatever reason when I do it the style of realtime it doesn't work... i tried a lot of stuff but whatevs
    m_shader = ShaderLoader::createShaderProgram("../../resources/shaders/default.vert", "../../resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram("../../resources/shaders/texture.vert", "../../resources/shaders/texture.frag");

//    resources/shaders/default.frag
//                              resources/shaders/default.vert

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
//    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glViewport(0, 0, DSCREEN_WIDTH, DSCREEN_HEIGHT);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader,"textureSample"),0);
    glUseProgram(0);

    float z=0.0;
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS  //        // UV //
            -1.0f,  1.0f, z,          0.f,1.f,
            -1.0f, -1.0f, z,          0.f,0.f,
            1.0f, 1.0f, z,           1.f,1.f,
            1.0f,  1.0f, z,          1.f,1.f,
            -1.0f,  -1.0f, z,         0.f,0.f,
            1.0f, -1.0f, z,          1.f,0.f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    resizeGL(DSCREEN_WIDTH, DSCREEN_HEIGHT);

    generateShapes();


    makeFBO();
}


void Renderer::makeFBO(){
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1,&m_fbo_texture);
    glBindTexture(GL_TEXTURE_2D,m_fbo_texture);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_fbo_width,m_fbo_height,0,GL_RGBA,GL_UNSIGNED_BYTE,nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D,0);

    glGenRenderbuffers(1,&m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER,m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,m_fbo_width,m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER,0);

    glGenFramebuffers(1,&m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_fbo_texture,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,m_fbo_renderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER,m_defaultFBO);
}

void inline initializeModelGeometry(Model& model) {
    for(RenderObject& shape : model.objects) {
        std::vector<GLfloat> vertexData;

        if(shape.primitive.type==PrimitiveType::PRIMITIVE_SPHERE){
            Sphere sphere;
            sphere.updateParams(settings.shapeParameter1,settings.shapeParameter2);
            vertexData = sphere.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CUBE){
            Cube cube;
            cube.updateParams(settings.shapeParameter1);
            vertexData = cube.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CONE){
            Cone cone;
            cone.updateParams(settings.shapeParameter1,settings.shapeParameter2);
            vertexData = cone.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CYLINDER){
            Cylinder cylinder;
            cylinder.updateParams(settings.shapeParameter1,settings.shapeParameter2);
            vertexData = cylinder.generateShape();
        } else {
            Trimesh trimesh;
            trimesh.updateParams(settings.shapeParameter1,settings.shapeParameter2,shape,shape.primitive.meshfile);
            vertexData = trimesh.generateShape();
        }

        shape.vertCount = vertexData.size() / 6;

        GLuint vbo;
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);

        shape.vbo = vbo;

        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

        GLuint vao;
        glGenVertexArrays(1,&vao);

        glBindVertexArray(vao);
        shape.vao = vao;

        glEnableVertexAttribArray(0);//vertexposition
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));

        glEnableVertexAttribArray(1);//vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
    }
}


void Renderer::bindBuff(std::vector<float>&& dat, int ind) {

    // Generate and bind VBO
    glGenBuffers(1, &(m_vbos[ind]));
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[ind]);
        //generateSphereData(10,20);

    m_data[ind] = dat;

    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_data[ind].size() * sizeof(GLfloat),
                 m_data[ind].data(), GL_STATIC_DRAW);

    // Generate, and bind vao
    glGenVertexArrays(1, &(m_vaos[ind]));
    glBindVertexArray(m_vaos[ind]);


    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

}
void Renderer::generateShapes() {

    Sphere sphere = Sphere();
    sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    bindBuff(sphere.generateShape(), sphere_in);

    Cube cube = Cube();
    cube.updateParams(settings.shapeParameter1);
    bindBuff(cube.generateShape(), cube_in);

    Cone cone = Cone();
    cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    bindBuff(cone.generateShape(), cone_in);

    Cylinder cyl = Cylinder();
    cyl.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    bindBuff(cyl.generateShape(), cylinder_in);

}


//void Renderer::initializeScene(std::string filepath) {
void Renderer::initializeScene(/*SceneParser* parser)*/) {
    parsed_ = SceneParser::hasParsed();//SceneParser::parse(filepath,m_metaData);

//    if (parser == nullptr)
//        std::cout << "null parser" << std::endl;


    if(!parsed_) {
        std::cerr << "Unparsed data" << std::endl;
        return;
    }

    m_metaData = SceneParser::getSceneData();

    m_camera = Camera(DSCREEN_WIDTH,DSCREEN_HEIGHT,m_metaData.cameraData);//Camera(size().width(),size().height(),m_metaData.cameraData);

    glUseProgram(m_shader);

    Lights lights(m_metaData.lights,m_metaData.lights.size());
    lights.clearUniformLightData(m_shader);
    lights.setUniformLightData(m_shader);

    glm::vec4 cameraPos = glm::vec4(m_camera.getPos(),0);
    glUniform4fv(glGetUniformLocation(m_shader,"worldSpaceCameraPos"),1,&cameraPos[0]);

    glUseProgram(0);

    //TODO: loop for all players in match; use data structure representing all players
    // All player rendering should happen locally imo. we should implement a network system to only
    //send player model ID, location, and rotation. that's all you need. - Luke
    m_player.generateGeometry();
    m_player.startAnimation();
    m_player.assignModelID(m_model_count++);
//    m_models.push_back(m_player.getModel());
//    m_models.push_back((Model){getModelObjectsList(m_metaData.shapes),m_model_count++});

    int obs = 0;
//    for (RenderObject& prim : m_metaData.shapes) {
//        Model mod;
//        std::vector<RenderObject>().swap(mod.objects);

////        RenderObject ob = ;
//        mod.objects.push_back(RenderObject(prim));
//        mod.id = obs;

////        m_models.push_back(mod);
//        obs++;
//    }

    for(Model& model : m_models) {
        initializeModelGeometry(model);
    }
    std::cout << "Renderer Init Complete" << std::endl;

}


void Renderer::beginFrame() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//moved to top
    glUseProgram(m_shader);
    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo);

    glViewport(0,0,m_fbo_width,m_fbo_height);

}

void Renderer::renderFrame() {
    paintGL();
}

void Renderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//moved to top
    glUseProgram(m_shader);
    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo);

    glViewport(0,0,m_fbo_width,m_fbo_height);

    paintGeometry();

    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER,m_defaultFBO);
    glViewport(0,0,m_fbo_width,m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(0);//moved from geometry

    // Call paintTexture to draw FBO color attachment texture
    paintTexture(m_fbo_texture,true);
}

void Renderer::paintTexture(GLuint texture, bool includePost){
    glUseProgram(m_texture_shader);

    GLint kernelFilter = settings.kernelBasedFilter ? 1 : 0;
    GLint pixelFilter = settings.perPixelFilter ? 1 : 0;
    glUniform1i(glGetUniformLocation(m_texture_shader,"kernel"),kernelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader,"perpixel"),pixelFilter);

    glBindVertexArray(m_fullscreen_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void paintModel(Model& model, GLuint shader) {
    for(RenderObject& shape : model.objects) {
        glBindVertexArray(shape.vao);
        glUniform1i(glGetUniformLocation(shader,"shininess"),shape.primitive.material.shininess);
        glUniform4fv(glGetUniformLocation(shader,"cAmbient"),1,&shape.primitive.material.cAmbient[0]);
        glUniform4fv(glGetUniformLocation(shader,"cDiffuse"),1,&shape.primitive.material.cDiffuse[0]);
        glUniform4fv(glGetUniformLocation(shader,"cSpecular"),1,&shape.primitive.material.cSpecular[0]);

        GLint modelLoc = glGetUniformLocation(shader, "modelMatrix");
        glUniformMatrix4fv(modelLoc,1,GL_FALSE,&shape.ctm[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, shape.vertCount);

        glBindVertexArray(0);
    }
}



void Renderer::paintGeometry() {

    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // TODO
    // Paramter support (just regenerate meshes on setting changes)
    // Multiple objects (generate VBO/VOA for each object, just a lot of code)
    // Multiple lights (may need to also pass in light color data, and type)

    // Draw to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    //    std::cout << m_fbo << std::endl;

    // Task 28: Call glViewport
    glViewport(0, 0, DSCREEN_WIDTH, DSCREEN_HEIGHT);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1f(glGetUniformLocation(m_shader,"k_a"),m_metaData.globalData.ka);
    glUniform1f(glGetUniformLocation(m_shader,"k_d"),m_metaData.globalData.kd);
    glUniform1f(glGetUniformLocation(m_shader,"k_s"),m_metaData.globalData.ks);


    glm::mat4 proj = m_camera.getPerspectiveMatrix(settings.nearPlane,settings.farPlane);
    glm::mat4 view = m_camera.getViewMatrix();


    GLint viewLoc = glGetUniformLocation(m_shader, "viewMatrix");
    GLint projLoc = glGetUniformLocation(m_shader, "projMatrix");
    glUniformMatrix4fv(viewLoc,1,GL_FALSE,&view[0][0]);
    glUniformMatrix4fv(projLoc,1,GL_FALSE,&proj[0][0]);


    for (RenderObject& sp : m_metaData.shapes) {

        int in = 0;

        switch (sp.primitive.type) {
        case PrimitiveType::PRIMITIVE_CUBE:
            in = cube_in;
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            in = sphere_in;
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            in = cylinder_in;
            break;
        case PrimitiveType::PRIMITIVE_CONE:
            in = cone_in;
            break;
        default:
            in = 0;
            break;
        }

        // Bind Sphere Vertex Data
        glBindVertexArray(m_vaos[in]);

//        setUniforms(sp);


        // NOT PROPERLY INTEGRATED INTO LIGHTING CODE YET

        int used = 0;
        /*for (int i = 0; i < m_metaData.lights.size(); i++) {

            SceneLightData lt = m_metaData.lights[i];

            //            if (lt.type != LightType::LIGHT_DIRECTIONAL)
            //                continue;

            //            float fa = std::min(1.f, 1.f/(lt->function.x + (dist * lt->function.y) + (sqr_dist * light->function.z)));
            // PASS IN all light data
            // direction, color, etc.
            GLint loc = glGetUniformLocation(m_shader, ("lights[" + std::to_string(used) + "]").c_str());
            //            std::cout << "pos " << lt.pos[0] << " " << lt.pos[1] << " " << lt.pos[2] << std::endl;
            glUniform4fv(loc, 1, &lt.pos[0]);


            //            GLint loc = glGetUniformLocation(m_shader, ("light_dist[" + std::to_string(used) + "]").c_str());
            //            glUniform1f(loc, glm::distance(lt.pos, sp.ctm * glm::vec4(0, 0, 0, 1)));

            GLint dir = glGetUniformLocation(m_shader, ("light_dir[" + std::to_string(used) + "]").c_str());
            glm::vec4 dir_vec = glm::normalize(lt.dir);
            glUniform4fv(dir, 1, &dir_vec[0]);

            GLint loc_fun = glGetUniformLocation(m_shader, ("light_fun[" + std::to_string(used) + "]").c_str());
            //            std::cout << "func: " << lt.function.x << " " << lt.function.y << " " << lt.function.z << std::endl;
            glUniform3fv(loc_fun, 1, &lt.function[0]);

            GLint loc_in = glGetUniformLocation(m_shader, ("light_in[" + std::to_string(used) + "]").c_str());
            glm::vec3 col_vec = lt.color;
            glUniform3fv(loc_in, 1, &col_vec[0]);

            GLint loc_type = glGetUniformLocation(m_shader, ("type[" + std::to_string(used) + "]").c_str());
            glUniform1i(loc_type, static_cast<std::underlying_type_t<LightType>>(lt.type));
            //            std::cout << "type " << static_cast<std::underlying_type_t<LightType>>(lt.type) << std::endl;
            if (loc_type == -1) {
                std::cout << "type not found" << std::endl;
            }

            GLint loc_angle = glGetUniformLocation(m_shader, ("angle[" + std::to_string(used) + "]").c_str());
            if (loc_angle == -1) {
                std::cout << "angle not found" << std::endl;
            }
            glUniform1f(loc_angle, lt.angle);

            GLint loc_pnum = glGetUniformLocation(m_shader, ("penumbra[" + std::to_string(used) + "]").c_str());
            if (loc_pnum == -1) {
                std::cout << "penum not found" << std::endl;
            }
            glUniform1f(loc_pnum, lt.penumbra);

            used++;

        }*/


//        GLint loc_in = glGetUniformLocation(m_shader, "light_num");
//        glUniform1i(loc_in, used);

//        if (loc_in == -1) {
//            std::cout << "light num err" << std::endl;
//        }

        // Actually draw geo
        glBindVertexArray(m_vaos[in]);
        glDrawArrays(GL_TRIANGLES, 0, m_data[in].size() / 6);

    }
    glBindVertexArray(0);
}

void Renderer::handleMovement(float deltaTime) {
    Movement mov(m_camera);
    float dist = 5.f * deltaTime;
    if (m_keyMap[Qt::Key_W]) {
        mov.forward(dist);
    } if(m_keyMap[Qt::Key_S]) {
        mov.backward(dist);
    } if(m_keyMap[Qt::Key_A]) {
        mov.left(dist);
    } if(m_keyMap[Qt::Key_D]) {
        mov.right(dist);
    } if(m_keyMap[Qt::Key_Space]) {
        mov.up(dist);
    } if(m_keyMap[Qt::Key_Control]) {
        mov.down(dist);
    }
}

void Renderer::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    //    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glDeleteTextures(1,&m_fbo_texture);
    glDeleteRenderbuffers(1,&m_fbo_renderbuffer);
    glDeleteFramebuffers(1,&m_fbo);

    m_fbo_width =w* m_devicePixelRatio;
    m_fbo_height = h * m_devicePixelRatio;
    glUseProgram(m_texture_shader);
    glUniform1i(glGetUniformLocation(m_texture_shader,"width"),m_fbo_width);
    glUniform1i(glGetUniformLocation(m_texture_shader,"height"),m_fbo_height);
    glUseProgram(0);

    makeFBO();
}

/*
void Renderer::sceneChanged() {
    resizeGL(size().width(),size().height());
    initializeScene(settings.sceneFilePath);
    update(); // asks for a PaintGL() call to occur
}*/

void Renderer::settingsChanged() {
    if(parsed_) {
        Lights lights(m_metaData.lights,8);
        lights.clearUniformLightData(m_shader);
        for(RenderObject& shape: m_metaData.shapes) {
            glDeleteBuffers(1,&shape.vbo);
            glDeleteVertexArrays(1,&shape.vao);
            shape.vbo = 0;
            shape.vao = 0;
        }
//        initializeScene(settings.sceneFilePath);
    }
//    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!
/*
void Renderer::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Renderer::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Renderer::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        m_camera.updateRotation(deltaX,deltaY);
        //        m_camera.rotateCam(deltaX,deltaY);

        update(); // asks for a PaintGL() call to occur
    }
}

void Renderer::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    handleMovement(deltaTime);
    m_player.stepLegs(deltaTime);

    update(); // asks for a PaintGL() call to occur
}
*/
// DO NOT EDIT
void Renderer::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
//    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
