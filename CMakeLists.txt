cmake_minimum_required(VERSION 3.16)

# Sets project name
project(quake-clone LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)

# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/ecs.cpp
    src/network/network.cpp

    src/core/ecs.h
    src/game_types.h
    src/network/network.h
    src/game.h src/game.cpp
    src/renderer/renderer.h src/renderer/renderer.cpp
    src/renderer/scenedata.h
    src/renderer/objects/player.h src/renderer/objects/player.cpp
    src/renderer/objects/cube.h src/renderer/objects/cube.cpp
    src/renderer/objects/sphere.h src/renderer/objects/sphere.cpp
    src/renderer/objects/cone.h src/renderer/objects/cone.cpp
    src/renderer/objects/cylinder.h src/renderer/objects/cylinder.cpp
    src/renderer/sceneparser.h src/renderer/sceneparser.cpp
    src/renderer/scenefilereader.h src/renderer/scenefilereader.cpp
    src/renderer/camera.h src/renderer/camera.cpp
    src/renderer/movement.h src/renderer/movement.cpp
    src/settings.h src/settings.cpp
    src/renderer/ShaderLoader.h
    src/renderer/lights.h src/renderer/lights.cpp
    src/renderer/objects/trimesh.h src/renderer/objects/trimesh.cpp
    src/renderer/objects/OBJ_Loader.h
    src/renderer/mainwindow.h src/renderer/mainwindow.cpp
    src/renderer/AspectRatioWidget.hpp
    resources/shaders/default.frag resources/shaders/default.vert resources/shaders/texture.frag resources/shaders/texture.vert
    src/renderer/rendermodel.h src/renderer/rendermodel.cpp


)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c

)
include_directories(${PROJECT_NAME} PRIVATE glew/include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    StaticGLEW
)

# Specifies other files
# qt6_add_resources(${PROJECT_NAME} "Resources"
#     PREFIX
#         "/"
#     FILES
#         resources/shaders/default.frag
#         resources/shaders/default.vert
# )

# target_sources(projects_realtime_other_files
#   PRIVATE
#     resources/shaders/texture.frag resources/shaders/texture.vert
# )

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()
