#include "rendermodel.h"
#include "objects/sphere.h"
#include "objects/cone.h"
#include "objects/cube.h"
#include "objects/trimesh.h"
#include "objects/sphere.h"
#include "objects/cylinder.h"
//#include "glfw/glfw3.h"


RenderModel::RenderModel(std::vector<RenderObject> objects, u_int8_t id) : m_objects(objects), m_id(id)
{

}


void RenderModel::initializeGLGeometry(int param1, int param2) {
    for(RenderObject& shape : m_objects) {
        std::vector<GLfloat> vertexData;

        if(shape.primitive.type==PrimitiveType::PRIMITIVE_SPHERE){
            Sphere sphere;
            sphere.updateParams(param1,param2);
            vertexData = sphere.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CUBE){
            Cube cube;
            cube.updateParams(param1);
            vertexData = cube.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CONE){
            Cone cone;
            cone.updateParams(param1,param2);
            vertexData = cone.generateShape();
        } else if(shape.primitive.type==PrimitiveType::PRIMITIVE_CYLINDER){
            Cylinder cylinder;
            cylinder.updateParams(param1,param2);
            vertexData = cylinder.generateShape();
        } else {
            Trimesh trimesh;
            trimesh.updateParams(param1,param2,shape,shape.primitive.meshfile);
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

void RenderModel::paintModel(GLuint shader) {
    for(RenderObject& shape : m_objects) {
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
