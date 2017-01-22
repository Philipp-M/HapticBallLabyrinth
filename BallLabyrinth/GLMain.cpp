#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <stdlib.h>
#include "GLMain.hpp"

GLMain::GLMain(SDL_Window *window, SDL_GLContext &context, std::vector<std::string> &objFilePaths,
               std::string &materialFolder) : objFilePaths(objFilePaths), materialFolder(materialFolder) {
    initializeGraphics(window, context);
}

GLMain::~GLMain() {
    cleanup();
}

void GLMain::cleanup() {
    glDeleteBuffers(1, &vbo);
}

void GLMain::initializeGraphics(SDL_Window *window, SDL_GLContext &context) {
    /********** OpenGL Context and GLEW **********/
    context = SDL_GL_CreateContext(window);

    GLenum rev;
    glewExperimental = GL_TRUE;
    rev = glewInit();

    if (GLEW_OK != rev) {
        std::cout << "Error: " << glewGetErrorString(rev) << std::endl;
        exit(1);
    }

    /********** VSYNC **********/
    SDL_GL_SetSwapInterval(1);

    /********** setup shader **********/
    shaderProgram.reset(new ShaderProgram("BallLabyrinth"));
    shaderProgram->attachShader(Shader("vertex", "shaders/defaultVs.glsl", ShaderType::VERTEX));
    shaderProgram->attachShader(Shader("fragment", "shaders/defaultFs.glsl", ShaderType::FRAGMENT));
    shaderProgram->link();
    shaderProgram->bind();

    /********** setup the drawing primitive **********/
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    scene.reset(new Scene(objFilePaths, materialFolder));

    /********** setup the initial translation for the ball **********/
    glm::vec3 initTranslation(-13.0, 2.0, -13.0);
    scene->getModelByName("Ball")->translate(initTranslation);
    glm::vec4 mirrorAxis(1.0, 1.0, -1.0, 1.0);
    scene->getModelByName("Labyrinth")->mirror(mirrorAxis);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void GLMain::reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void GLMain::display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene->draw(*shaderProgram);

    /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram->bind();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);*/
}

void GLMain::rotateModelAroundAxis(int id, int axis, float angle) {
    scene->rotateModelAroundAxis(id, axis, angle);
}

void GLMain::resetModelRotationAroundAxis(int id) {
    scene->resetModelRotationAroundAxis(id);
}

const std::shared_ptr<Scene> &GLMain::getScene() const {
    return scene;
}



