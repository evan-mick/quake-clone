#include "trimesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/gtx/normal.hpp>
#include "OBJ_Loader.h"

void Trimesh::updateParams(int param1, int param2, RenderObject prim, std::string file) {
    m_vertexData.clear();
    m_param1 = param1;
    m_param2 = param2;
    m_prim = prim;
    m_meshfile = file;
    setVertexData();
}



void Trimesh::setVertexData() {
    makeMesh();
}

void Trimesh::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Trimesh::makeMesh() {
    objl::Loader loader;
    loader.LoadFile(m_meshfile);

    for(int i=0;i<loader.LoadedVertices.size();i++) {
        insertVec3(m_vertexData,glm::vec3(loader.LoadedVertices[i].Position.X,
                                           loader.LoadedVertices[i].Position.Y,
                                           loader.LoadedVertices[i].Position.Z));
        insertVec3(m_vertexData,glm::vec3(loader.LoadedVertices[i].Normal.X,
                                           loader.LoadedVertices[i].Normal.Y,
                                           loader.LoadedVertices[i].Normal.Z));
    }
}
