#include "Renderer.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "renderer/objects/trimesh.h"
#include "shaderloader.h"
#include "objects/Sphere.h"
#include "objects/Cube.h"
#include "objects/Cylinder.h"
#include "objects/Cone.h"
#include "scene/sceneparser.h"
#include "game_types.h"
#include <QString>
#include <QFileInfo>

#include "glm/gtx/transform.hpp"



// ================== Project 5: Lights, Camera

Renderer::Renderer(Camera* cam, bool fullSetup)
    //: QOpenGLWidget(parent)
{

    if (!fullSetup)
        return;

    camera = cam;
    m_models = generateModelsMap();

    m_prev_mouse_pos = glm::vec2(DSCREEN_WIDTH/2, DSCREEN_HEIGHT/2);
    //setMouseTracking(true);
    //setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
    data = &SceneParser::getSceneData();
    initializeGL();

    // If you must use this function, do not edit anything above this

}


void Renderer::finish() {


//    killTimer(m_timer);
//    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    // Delete VAO and VBOs, delete the program
//    glDeleteProgram()
    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);

    glDeleteBuffers(1, &m_fbo);
    glDeleteBuffers(1, &m_defaultFBO);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);

    glDeleteBuffers(5, &(m_vbos[0]));
    glDeleteVertexArrays(5, &(m_vaos[0]));

    glDeleteTextures(1, &m_fbo_texture);

    for (auto it = m_texture_map.begin(); it != m_texture_map.end(); ++it) {
        glDeleteTextures(1,&it->second.tex);
    }


//    this->doneCurrent();
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


void Renderer::makeFBO() {
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Renderer::initializeGL() {

    old_settings = settings;

    std::cout <<" HII" << std::endl;

//    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();



    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);



    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_defaultFBO = 0;

    m_screen_width = DSCREEN_WIDTH;
    m_screen_height =  DSCREEN_HEIGHT;

    m_o_scrn_width = m_screen_width;
    m_o_scrn_height = m_screen_height;

    // Tells OpenGL how big the screen is
    glViewport(0, 0, m_screen_width, m_screen_height);
    m_models = generateModelsMap();


    m_shader = ShaderLoader::createShaderProgram("../../resources/shaders/default.vert", "../../resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram("../../resources/shaders/texture.vert", "../../resources/shaders/texture.frag");
    m_skybox_shader = ShaderLoader::createShaderProgram("../../resources/shaders/skybox.vert", "../../resources/shaders/skybox.frag");

    glUseProgram(m_texture_shader);
    GLint texLoc = glGetUniformLocation(m_texture_shader, "tex");
    if (texLoc == -1) {
        std::cout << "err " << m_texture_shader << std::endl;
    }
    glUniform1i(texLoc, 0);

    m_texture_map = generateTexturesMap();
    loadTextures();

    // FULLSCREEN QUAD CODE
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS, THEN UV's    //
            -1.f,  1.f, 0.0f,
            0.f,  1.f, 0.0f,

            -1.f, -1.f, 0.0f,
            0.0f, 0.0f, 0.0f,

            1.f, -1.f, 0.0f,
            1.f, 0.0f, 0.0f,


            1.f, -1.f, 0.0f,
            1.f,  0.f, 0.0f,


            1.f,  1.f, 0.0f,
            1.f,  1.f, 0.0f,


            -1.f,  1.f, 0.0f,
            0.f, 1.f, 0.0f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    generateShape();

    init_gen = true;

    makeFBO();

    loadSkyboxTexture();

    sceneChanged();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr <<  "OpenGL error end of initialize: " << error << std::endl;
    }
}



std::map<u_int8_t,Model> Renderer::generateModelsMap() {
    std::map<u_int8_t,Model> models;

    SceneMaterial level_mat;

    level_mat.cAmbient = glm::vec4(0.5,0.5,0.5,1);
    level_mat.cDiffuse = glm::vec4(.5);
    level_mat.cSpecular = glm::vec4(.5);
    level_mat.shininess = 1.0;

    ScenePrimitive level_cube_prim = {.type = PrimitiveType::PRIMITIVE_CUBE,.material=level_mat};
    ScenePrimitive level_cone_prim = {.type = PrimitiveType::PRIMITIVE_CONE,.material=level_mat};
    ScenePrimitive level_cyl_prim = {.type = PrimitiveType::PRIMITIVE_CYLINDER,.material=level_mat};
    ScenePrimitive level_sphere_prim = {.type = PrimitiveType::PRIMITIVE_SPHERE,.material=level_mat};

    RenderObject level_cube = {.primitive=level_cube_prim,.ctm=glm::mat4(1.f)};
    RenderObject level_cone = {.primitive=level_cone_prim,.ctm=glm::mat4(1.f)};
    RenderObject level_cyl = {.primitive=level_cyl_prim,.ctm=glm::mat4(1.f)};
    RenderObject level_sphere = {.primitive=level_sphere_prim,.ctm=glm::mat4(1.f)};

    models.insert({0,(Model){.objects=std::vector<RenderObject>(1,level_cube)}});
    models.insert({1,(Model){.objects=std::vector<RenderObject>(1,level_cone)}});
    models.insert({2,(Model){.objects=std::vector<RenderObject>(1,level_cyl)}});
    models.insert({3,(Model){.objects=std::vector<RenderObject>(1,level_sphere)}});

    Player player;
    player.generateGeometry();
    models.insert({5,player.getModel()});

    std::cout <<"Model map generated."<<std::endl;
    return models;
}

std::map<QString,SceneTexture> Renderer::generateTexturesMap() {
    std::map<QString,SceneTexture> res;

    QString crosshair_path("../../resources/textures/crosshair.png");
    SceneTexture crosshair;
    crosshair.width = 1600;
    crosshair.height = 1200;
    crosshair.image = QImage(crosshair_path).convertToFormat(QImage::Format_RGBA8888).mirrored();
    res.insert({crosshair_path,crosshair});
    return res;
}

void Renderer::loadSkyboxTexture() {

    int slot = 4;
    glUseProgram(m_skybox_shader);
    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &m_skybox_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox_texture);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr <<  "OpenGL error before loop: " << error << std::endl;
    }

    for (GLuint i = 0; i < 6; i++) {
        QString facePath = QString("../../resources/textures/cloudbox") + QString::number(i) + ".png";
        QImage faceImage(facePath);
        QImage faceImageConv = faceImage.convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                     faceImageConv.width(), faceImageConv.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, faceImageConv.bits());
    }
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr <<  "OpenGL error after loop load: " << error << std::endl;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenVertexArrays(1, &m_skybox_vao);
    glBindVertexArray(m_skybox_vao);

    for(int i=0;i<108;i++) {
        skybox_vertices[i] = skybox_vertices[i] * 200.f;
    }

    glGenBuffers(1, &m_skybox_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), skybox_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUniform1i(glGetUniformLocation(m_skybox_shader, "skybox"), slot);

    glUseProgram(0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr <<  "OpenGL error in load: " << error << std::endl;
    }
}

void Renderer::paintSkybox() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_skybox_shader);
    glActiveTexture(GL_TEXTURE4);

    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox_texture);

    glBindVertexArray(m_skybox_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
//    GLenum error = glGetError();
//    if (error != GL_NO_ERROR) {
//        std::cerr <<  "OpenGL error in paint: " << error << std::endl;
//    }
}

void Renderer::loadTextures() {
    int slot=1;
    std::cout << "size: " << m_texture_map.size() << std::endl;
    std::cout << "rx"<<ratio_x << std::endl;
    std::cout << "ry"<<ratio_y << std::endl;
    glUseProgram(m_texture_shader);


    for (auto it = m_texture_map.begin(); it != m_texture_map.end(); ++it) {
        SceneTexture *texture = &it->second;
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1,&texture->tex);
        glBindTexture(GL_TEXTURE_2D,texture->tex);
//        texture->image = texture->image.scaled(ratio_x*1600,ratio_y*1200);
//        texture->image = texture->image.scaled(m_screen_width,m_screen_height);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,texture->image.width(),texture->image.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture->image.bits());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D,0);

        texture->slot = slot;

        std::cout <<"image width: "<<texture->image.width() << std::endl;

        //pass texture slot uniform
        QFileInfo fileInfo(it->first);
        QString filename = fileInfo.fileName();
        filename.chop(4);

        //uniform name is image path w/o file extension name
        GLint texLoc = glGetUniformLocation(m_texture_shader, "tex");
        if (texLoc == -1) {
            std::cout << "err loading " << filename.toStdString()<< "," << m_texture_shader << std::endl;
        } else {
            std::cout << "loaded " << filename.toStdString()<< " into slot " << slot << std::endl;
        }
        glUniform1i(texLoc, slot);
        slot++;

    }
    glUseProgram(0);
}



void Renderer::paintTexture(GLuint texture, bool post_process, float opacity){
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_texture_shader);

    glUniform1i(glGetUniformLocation(m_texture_shader, "filt"), settings.perPixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "box_blur"), settings.kernelBasedFilter);

    glm::vec2 uv_traverse = glm::vec2(1.f / m_o_scrn_width, 1.f / m_o_scrn_height);
    glUniform2fv(glGetUniformLocation(m_texture_shader, "uv"), 1, &uv_traverse[0]);
    glUniform1f(glGetUniformLocation(m_texture_shader, "opacity"), 0.5f);

    glBindVertexArray(m_fullscreen_vao);

//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, texture);


//    glDrawArrays(GL_TRIANGLES, 0, 6);



//    glBindTexture(GL_TEXTURE_2D, 0);
//    glDisable(GL_BLEND);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,m_texture_map.begin()->second.tex);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);


}

void Renderer::paintTexture(GLuint texture, bool post_process, int slot, float opacity){
    if(slot==0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    glUseProgram(m_texture_shader);

    glUniform1i(glGetUniformLocation(m_texture_shader, "tex"), slot);
    glUniform1i(glGetUniformLocation(m_texture_shader, "filt"), settings.perPixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "box_blur"), settings.kernelBasedFilter);

    glm::vec2 uv_traverse = glm::vec2(1.f / m_o_scrn_width, 1.f / m_o_scrn_height);
    glUniform2fv(glGetUniformLocation(m_texture_shader, "uv"), 1, &uv_traverse[0]);

    glBindVertexArray(m_fullscreen_vao);

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Task 33: Pass the opacity value to the shader
    glUniform1f(glGetUniformLocation(m_texture_shader, "opacity"), opacity);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);


    if(slot>=m_texture_map.size()) {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
}




void Renderer::generateShape() {

    int p_1 = 10;
    int p_2 = 10;

    Sphere sphere = Sphere();
    sphere.updateParams(p_1, p_2);
    bindBuff(sphere.generateShape(), sphere_in);

    Cube cube = Cube();
    cube.updateParams(p_1);
    bindBuff(cube.generateShape(), cube_in);

    Cone cone = Cone();
    cone.updateParams(p_1, p_2);
    bindBuff(cone.generateShape(), cone_in);

    Cylinder cyl = Cylinder();
    cyl.updateParams(p_1, p_2);
    bindBuff(cyl.generateShape(), cylinder_in);

    Trimesh mesh = Trimesh();
    mesh.updateParams(p_1,p_2,"../../resources/meshes/stretch obama.obj");
    bindBuff(mesh.generateShape(), trimesh_in);
}

void Renderer::setUniforms(RenderObject& sp) {

    glm::vec4 cam_pos = glm::inverse(camera->getViewMatrix()) * glm::vec4(0, 0, 0, 1);//inverse(m_view) * glm::vec4(0, 0, 0, 1);

    GLint matrixLoc = glGetUniformLocation(m_shader, "model_matrix");
    GLint viewLoc = glGetUniformLocation(m_shader, "view_matrix");
    GLint projLoc = glGetUniformLocation(m_shader, "proj_matrix");
    if (viewLoc == -1 || projLoc == -1 || matrixLoc == -1) {
        std::cout << "matrix name wrong" << std::endl;
        exit(1);
    }
    // OBJECT SPACE NORMAL SOMEWHERE
    auto view_mat = camera->getViewMatrix();
    auto proj_mat = camera->getProjectionMatrix();

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &(view_mat[0][0]));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &(proj_mat[0][0]));


    glUniform4f(glGetUniformLocation(m_shader, "ambient"),
                sp.primitive.material.cAmbient.x,
                sp.primitive.material.cAmbient.y,
                sp.primitive.material.cAmbient.z,
                sp.primitive.material.cAmbient[3]);

    glUniform4f(glGetUniformLocation(m_shader, "diffuse"),
                sp.primitive.material.cDiffuse.x,
                sp.primitive.material.cDiffuse.y,
                sp.primitive.material.cDiffuse.z,
                sp.primitive.material.cDiffuse[3]);

    glUniform4f(glGetUniformLocation(m_shader, "specular"),
                sp.primitive.material.cSpecular.x,
                sp.primitive.material.cSpecular.y,
                sp.primitive.material.cSpecular.z,
                sp.primitive.material.cSpecular[3]);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before 4fv uniform error: " << error << std::endl;
    }

    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &(sp.ctm[0][0]));
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL afterf 4fv uniform error: " << error << std::endl;
    }



    // Phong lighting constants
    GLuint ka_in = glGetUniformLocation(m_shader, "kd");
    if (ka_in == -1) {
        std::cout << "ka err" << std::endl;
    }

    if (data) {
        glUniform1f(glGetUniformLocation(m_shader, "ka"), data->globalData.ka);
        glUniform1f(ka_in, data->globalData.kd);
        glUniform1f(glGetUniformLocation(m_shader, "ks"), data->globalData.ks);
    }


    // PASS IN MATERIAL PARAMETERS, such as ambient, diffuse, specular, shininess from material

    glUniform1f(glGetUniformLocation(m_shader, "shininess"), sp.primitive.material.shininess);

    glUniform4f(glGetUniformLocation(m_shader, "cam_pos"),
                cam_pos.x, cam_pos.y, cam_pos.z, cam_pos[3]);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before skybox uniform error: " << error << std::endl;
    }

    glUseProgram(m_skybox_shader);
    //pass view and proj to skybox
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "view"), 1, GL_FALSE, &view_mat[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "projection"), 1, GL_FALSE, &proj_mat[0][0]);
    glUseProgram(m_shader);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL after skybox uniform error: " << error << std::endl;
    }
}

void Renderer::drawDynamicOb(struct ECS* e, entity_t ent, float delta_seconds) {
    Transform* trans = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
    Renderable* rend = static_cast<Renderable*>(e->getComponentData(ent, FLN_RENDER));

    if(rend->model_id==5) {//for player entity
        Player p;
        p.transformPlayer(trans);
        for (RenderObject& ob : p.getModel().objects) {
            drawRenderOb(ob);
        }
    } else {//for single-prim models associated
        Model mod = m_models[rend->model_id];

        for(RenderObject& ob : mod.objects) {

    //    RenderObject ob;
    //    ob.ctm = glm::mat4(1);

        ob.ctm = glm::translate(ob.ctm, trans->pos);
        glm::rotate(ob.ctm, trans->rot.x, glm::vec3(1, 0, 0));
        glm::rotate(ob.ctm, trans->rot.y, glm::vec3(0, 1, 0));
        glm::rotate(ob.ctm, trans->rot.z, glm::vec3(0, 0, 1));
        glm::scale(ob.ctm, trans->scale);

    //    ob.primitive.type = static_cast<PrimitiveType>(rend->model_id);
    //    ob.primitive.material = SceneMaterial();
    //    ob.primitive.material.cAmbient = SceneColor(1, 1, 1, 1);
    //    ob.primitive.material.cDiffuse = SceneColor(1, 1, 1, 1);
    //    ob.primitive.material.cReflective = SceneColor(1, 1, 1, 1);
    //    ob.primitive.material.cSpecular = SceneColor(1, 1, 1, 1);


        drawRenderOb(ob);
        }
    }


//    default_render.drawRenderOb();
}

void Renderer::queueDynamicModel(struct ECS* e, entity_t ent, float delta_seconds) {
    Transform* trans = static_cast<Transform*>(e->getComponentData(ent, FLN_TRANSFORM));
    Renderable* rend = static_cast<Renderable*>(e->getComponentData(ent, FLN_RENDER));


    if(rend->model_id==5) {//for player entity
        Player p;
        p.transformPlayer(trans);
        int i=0;
        for (RenderObject ob : p.getModel().objects) {
            ob.i =i;
            ob.ent = ent;
            auto found = std::find(m_dynamics.begin(),m_dynamics.end(),ob);
            if (found == m_dynamics.end()){
                m_dynamics.push_back(ob);
            }
            else {
                *found = ob;
            }
            i++;
        }
    } else {//for single-prim models associated
        Model mod = m_models[rend->model_id];
        int i=0;
        for(RenderObject& ob : mod.objects) {

        ob.ctm = glm::translate(ob.ctm, trans->pos);
        glm::rotate(ob.ctm, trans->rot.x, glm::vec3(1, 0, 0));
        glm::rotate(ob.ctm, trans->rot.y, glm::vec3(0, 1, 0));
        glm::rotate(ob.ctm, trans->rot.z, glm::vec3(0, 0, 1));
        glm::scale(ob.ctm, trans->scale);
        ob.i = i;
        ob.ent = ent;
        auto found = std::find(m_dynamics.begin(),m_dynamics.end(),ob);
        if (found == m_dynamics.end()){
            m_dynamics.push_back(ob);
        }
        else {
            *found = ob;
        }
            i++;
        }
    }
}

void Renderer::drawScreen() {

//    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
//    glViewport(0, 0, m_screen_width, m_screen_height);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    paintSkybox();
////
//    paintTexture(m_fbo_texture, true,0,1.f);
//    for (auto it = m_texture_map.begin(); it != m_texture_map.end(); ++it) {
//        paintTexture(it->second.tex, false,it->second.slot,1.f);
//    }

////    GLenum error = glGetError();
////    if (error != GL_NO_ERROR) {
////        std::cerr << "OpenGL error: " << error << std::endl;
////    }
//    glUseProgram(0);

    // Enable blending for overlapping objects
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr <<  "OpenGL error before load: " << error << std::endl;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Draw to screen
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    paintSkybox();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL draw screen error: " << error << std::endl;
    }

    // Clear the framebuffer
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    drawDynamicAndStaticObs();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL after draw obs error: " << error << std::endl;
    }

    // Draw the FBO texture with full opacity
    paintTexture(m_fbo_texture, true, 0, 1.0f);

    // Draw other textures with full opacity
    for (auto it = m_texture_map.begin(); it != m_texture_map.end(); ++it) {
        paintTexture(it->second.tex, false, it->second.slot, 1.0f);
    }

    // Disable blending for subsequent rendering
    glDisable(GL_BLEND);

    // Disable depth testing for subsequent rendering
    glDisable(GL_DEPTH_TEST);

    // Check for OpenGL errors


    // Reset the shader program
    glUseProgram(0);


    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL end draw screen error: " << error << std::endl;
    }
}

void Renderer::drawRenderOb(RenderObject& to_draw) {

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before switch error: " << error << std::endl;
    }
    int in = 0;

    switch (to_draw.primitive.type) {
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
    case PrimitiveType::PRIMITIVE_MESH:
        in = trimesh_in;
        break;
    }

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before binding error: " << error << std::endl;
    }

    // Bind Sphere Vertex Data
    glBindVertexArray(m_vaos[in]);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL after binding error: " << error << std::endl;
    }

    setUniforms(to_draw);


    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL after set uniform error: " << error << std::endl;
    }
    // NOT PROPERLY INTEGRATED INTO LIGHTING CODE YET

    int used = 0;
    if (data) {
        for (int i = 0; i < data->lights.size(); i++) {

            SceneLightData lt = data->lights[i];

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

        }
    }


    GLint loc_in = glGetUniformLocation(m_shader, "light_num");
    glUniform1i(loc_in, used);

    if (loc_in == -1) {
        std::cout << "light num err" << std::endl;
    }

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before actually drawing error: " << error << std::endl;
    }
    // Actually draw geo
    glBindVertexArray(m_vaos[in]);
    glDrawArrays(GL_TRIANGLES, 0, m_data[in].size() / 6);
    glBindVertexArray(0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL after actually drawing error: " << error << std::endl;
    }

}


void Renderer::startDraw() {
    // Draw to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    //    std::cout << m_fbo << std::endl;

    // Task 28: Call glViewport
    glViewport(0, 0, m_screen_width, m_screen_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Task 2: activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_shader);

}
void Renderer::drawStaticObs()
{ //(OBSOLETE)
    if (m_mouseDown) {
        // rotate cam based on delta x
    }

    if (!data)
        return;


    for (RenderObject sp : data->shapes) {
        drawRenderOb(sp);
    }
}

void Renderer::drawDynamicObs() {//(OBSOLETE)
    std::vector<RenderObject> toRender = m_dynamics;
    toRender.insert(toRender.end(),data->shapes.begin(),data->shapes.end());
    for (RenderObject& ob : toRender) {
        drawRenderOb(ob);

    }
}

void Renderer::drawDynamicAndStaticObs() {//USED
    // Render static objects first
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL before start draw func: " << error << std::endl;
    }
    glDepthRange(0.7, 1.0);
    for (RenderObject& ob : data->shapes) {
        drawRenderOb(ob);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL in start draw func: " << error << std::endl;
        }
    }


    glDepthFunc(GL_LESS);
    // Set a new depth range for dynamic objects
    glDepthRange(0.0, 0.4);

    // Render dynamic objects
    for (RenderObject& ob : m_dynamics) {
        drawRenderOb(ob);
    }
}


void Renderer::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
//    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

//    m_screen_width = size().width() * m_devicePixelRatio;
//    m_screen_height = size().height() * m_devicePixelRatio;
    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_screen_width = w;
    m_screen_height = h;

    glViewport(0, 0, m_screen_width, m_screen_height);

//    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);



}

void Renderer::sceneChanged() {

//    SceneParser parser = SceneParser();
//    parser.parse(settings.sceneFilePath);
//    parser.parse("../../resources/scenes/phong_total.json");

    if (!SceneParser::hasParsed()) {
        std::cout << "SCENE PARSER RENDERER ERROR" << std::endl;
        return;
    }
//    data = &SceneParser::getSceneData();
//    m_level.generateLevel();
//    for(Model& mod : m_level.getLevelModels()) {
//        for(RenderObject obj : mod.objects) {
//            data->shapes.push_back(obj);
//        }
//    }
//    data = SceneParser::getSceneData();


//    *camera = Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, m_level.);
//    std::exchange(camera, Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, data.cameraData));

//    camera = Camera(data.cameraData.up, data.cameraData.pos,
//                    data.cameraData.look,
//                    (float)size().width() / (float)size().height(),
//                    data.cameraData.aperture,
//                    data.cameraData.focalLength,
//                    data.cameraData.heightAngle,
//                    settings.nearPlane, settings.farPlane);

//    for (SceneData sp : data.shapes) {
//        std::cout << "SHAPE" << std::endl;
//    }

//    update(); // asks for a PaintGL() call to occur
}

void Renderer::settingsChanged() {

//    if (old_settings.nearPlane != settings.nearPlane || old_settings.farPlane != settings.farPlane)
//        camera.setNearFar(settings.nearPlane, settings.farPlane);

    if (init_gen && (old_settings.shapeParameter1 != settings.shapeParameter1 || old_settings.shapeParameter2 != settings.shapeParameter2))
        generateShape();

    old_settings = settings;

//    update(); // asks for a PaintGL() call to occur
}



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
    drawStaticObs();

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

