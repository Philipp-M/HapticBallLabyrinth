#pragma once


#include "ShaderProgram.hpp"
#include "Scene.hpp"
#include <SDL2/SDL.h>
#include <memory>

class GLMain {
private:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<ShaderProgram> shaderProgram;
    GLuint vao;
    GLuint vbo;

    std::string objFilePath, materialFolder;

    void initialize(SDL_Window *window, SDL_GLContext &context);

    void setUpDataBuffers();

public:
    GLMain(SDL_Window *window, SDL_GLContext &context, std::string objFilePath, std::string materialFolder);

    ~GLMain();

    void cleanup();

    void display();

    void reshape(int width, int height);

    void rotateModel(int id, int axis, float angle);

};

