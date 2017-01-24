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

    std::vector<std::string> objFilePaths;
    std::string materialFolder;

    void initializeGraphics(SDL_Window *window, SDL_GLContext &context);

public:
    GLMain(SDL_Window *window, SDL_GLContext &context, std::vector<std::string> &objFilePaths, std::string &materialFolder);

    ~GLMain();

    void clearOpenGLSceneAndShaderProgram();

    void initializeNewLabyrinth(std::string &objLabyrinthPath, std::string &materialFolder);

    void display();

    void reshape(int width, int height);

    void rotateModelAroundAxis(int id, int axis, float angle);

    void resetModelRotationAroundAxis(int id);

    const std::shared_ptr<Scene> &getScene() const;
};

