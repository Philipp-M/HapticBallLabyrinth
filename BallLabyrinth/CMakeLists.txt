cmake_minimum_required(VERSION 2.8)
project(BallLabyrinth)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# find SDL2 and SDL2_image
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
#link_directories(${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY})

# find OpenGL
find_package(OpenGL REQUIRED)
#include_directories(${OpenGL_INCLUDE_DIRS})
#link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

# find GLEW
find_package(GLEW REQUIRED)
#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES})

# find GLM
find_package(GLM REQUIRED)

set(SOURCE_FILES Shader.cpp
        include/Shader.hpp
        ShaderProgram.cpp
        include/ShaderProgram.hpp
        GLMain.cpp
        include/GLMain.hpp
        tiny_obj_loader.cpp
        include/tiny_obj_loader.hpp
        Model.cpp
        include/Model.hpp
        Scene.cpp
        include/Scene.hpp
        MaterialManager.cpp
        include/MaterialManager.hpp
        Camera.cpp
        include/Camera.hpp
        PointLight.cpp
        include/PointLight.hpp)

add_executable(BallLabyrinth main.cpp ${SOURCE_FILES})

add_executable(testScene testMain.cpp ${SOURCE_FILES})

target_include_directories(BallLabyrinth PUBLIC
        include
        ${GLM_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIR})

target_link_libraries(BallLabyrinth
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        ${GLEW_LIBRARIES}
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARY})

target_include_directories(testScene PUBLIC
        include
        ${GLM_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIR})

target_link_libraries(testScene
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        ${GLEW_LIBRARIES}
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARY})
