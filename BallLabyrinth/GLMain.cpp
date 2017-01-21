#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <stdlib.h>
#include "GLMain.hpp"

GLMain::GLMain(SDL_Window *window, SDL_GLContext &context, std::string objFilePath,
               std::string materialFolder) : objFilePath(objFilePath), materialFolder(materialFolder) {
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
    scene.reset(new Scene(objFilePath, materialFolder));

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

const std::shared_ptr<Scene> &GLMain::getScene() const {
    return scene;
}


