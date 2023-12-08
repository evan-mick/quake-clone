#include "level.h"

Level::Level(float x, float y, float z) : size_x(x), size_z(z), height(y)
{
//    m_objs = (RenderObject *)malloc(PRIM_COUNT * sizeof(RenderObject));
    m_mat.cAmbient = glm::vec4(0.5,0.5,0.5,1);
    m_mat.cDiffuse = glm::vec4(.5);
    m_mat.cSpecular = glm::vec4(.5);
    m_mat.shininess = 1.0;
}

void Level::generateLevel() {
    makeWalls();
    makeFloor();
    makeObstacles();
    makeSpawnpoints(2);
}

void Level::makeWalls() {
    glm::mat4 negZ = glm::translate(glm::vec3(0,height/2,-size_z/2.0));
    negZ = glm::scale(negZ,glm::vec3(size_x,height,1));
    insertSimpleModel(negZ,PrimitiveType::PRIMITIVE_CUBE,0);

    glm::mat4 posZ = glm::translate(glm::vec3(0,height/2,size_z/2.0));
    posZ = glm::scale(posZ,glm::vec3(size_x,height,1));
    insertSimpleModel(posZ,PrimitiveType::PRIMITIVE_CUBE,1);

    glm::mat4 negX = glm::translate(glm::vec3(-size_x/2.0,height/2,0));
    negX = glm::scale(negX,glm::vec3(1,height,size_z));
    insertSimpleModel(negX,PrimitiveType::PRIMITIVE_CUBE,2);

    glm::mat4 posX = glm::translate(glm::vec3(size_x/2.0,height/2,0));
    posX = glm::scale(posX,glm::vec3(1,height,size_z));
    insertSimpleModel(posX,PrimitiveType::PRIMITIVE_CUBE,3);

}

void Level::makeFloor() {
    glm::mat4 floor = glm::translate(glm::vec3(0,-0.5,0));
    floor = glm::scale(floor,glm::vec3(size_x,1,size_z));
    insertSimpleModel(floor,PrimitiveType::PRIMITIVE_CUBE,4);
}

void Level::makeObstacles() {
    float obst_size = 2;
    glm::mat4 obst_1 = glm::translate(glm::vec3(-5,obst_size/2,5));
    obst_1 = glm::scale(obst_1,glm::vec3(obst_size,obst_size,obst_size));

    glm::mat4 obst_2 = glm::translate(glm::vec3(5,obst_size/2,-5));
    obst_2 = glm::scale(obst_2,glm::vec3(obst_size,obst_size,obst_size));

    insertSimpleModel(obst_1,PrimitiveType::PRIMITIVE_CUBE,5);
    insertSimpleModel(obst_2,PrimitiveType::PRIMITIVE_CUBE,6);
}

std::vector<Model> Level::getLevelModels() {
    std::vector<Model> res;
    for(int i=0;i<PRIM_COUNT;i++) {
        std::vector<RenderObject *> objs;
        objs.push_back(&m_geometry[i]);
        res.push_back((Model){.objects=objs});
    }
    return res;
}

void Level::makeSpawnpoints(int count) {
    m_spawnpoints.push_back(glm::vec3(-10,0,-10));
    m_spawnpoints.push_back(glm::vec3(10,0,10));
}

glm::vec3 Level::getRandomSpawnpointPos() {
    std::random_device rd;
    std::mt19937 rng(rd());

    std::uniform_int_distribution<int> distribution(0, m_spawnpoints.size() - 1);
    int randomIndex = distribution(rng);

    return m_spawnpoints[randomIndex];
}


/**
 * @brief Level::insertSimpleModel inserts a single primitive Model struct into m_models,
 *          while also inserting its referenced object into m_objs (remember that Model struct
 *          only contains a list of pointers to RenderObjects; m_objs is where the actual
 *          primitive objects are stored).
 * @param ctm the cumulative transformation matrix of the primitive
 * @param type the primtive type
 */
void Level::insertSimpleModel(glm::mat4 ctm, PrimitiveType type, int index) {
    RenderObject shape;
    shape.primitive = {.type=type,.material=m_mat};
    shape.ctm = ctm;
    m_geometry[index] = shape;
}
