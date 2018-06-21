cmake_minimum_required(VERSION 2.8)
project(BallLabyrinth)

if (APPLE)
    set(CMAKE_CXX_COMPILER "g++-8")
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

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

# find BOOST
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
 
find_package(Boost 1.36.0 COMPONENTS thread filesystem system)

set(SOURCE_FILES Shader.cpp
        include/SerialCommunication.hpp
        include/HandleInterface.hpp
        HandleInterface.cpp
        include/Shader.hpp
        ShaderProgram.cpp
        include/ShaderProgram.hpp
        GLMain.cpp
        include/GLMain.hpp
        tiny_obj_loader.cpp
        include/tiny_obj_loader.hpp
        GraphicsModel.cpp
        include/GraphicsModel.hpp
        Scene.cpp
        include/Scene.hpp
        MaterialManager.cpp
        include/MaterialManager.hpp
        Camera.cpp
        include/Camera.hpp
        PointLight.cpp
        include/PointLight.hpp include/Physics.hpp Physics.cpp)

add_executable(BallLabyrinth main.cpp ${SOURCE_FILES})

target_include_directories(BallLabyrinth PUBLIC
        include
        ${GLM_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${OpenGL_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        )

target_link_libraries(BallLabyrinth
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARY}
        ${GLEW_LIBRARIES}
        ${SDL2_LIBRARY}
        ${SDL2_IMAGE_LIBRARY}
        ${Boost_LIBRARIES}
        )
