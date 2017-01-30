#pragma once


#include "ShaderProgram.hpp"
#include "Scene.hpp"
#include <SDL2/SDL.h>
#include <memory>

/**
 * Class handling the graphics.
 */
class GLMain {
private:
    std::shared_ptr<Scene> scene; /**< Scene object, containing the models and the scene. */
    std::shared_ptr<ShaderProgram> shaderProgram; /**< Shader program, used to render the scene. */
    GLuint vao; /**< vertex array object, not used, but necessary to be initialized for openGL. */

    std::vector<std::string> objFilePaths; /**< Paths to the oject files used for this scene. */
    std::string materialFolder; /**< Path to the folder in which the material files are stored. */

    /**
     * Function creates the OpenGL context, initializes GLEW, enables VSYNC if possible, compiles and linkes shader to
     * get the shader program, loads all primitives by creating a new Scene object and sets the initial translation of
     * the ball.
     * @param window window object in which the rendered images are drawn.
     * @param context OpenGL context which is related to the window.
     */
    void initializeGraphics(SDL_Window *window, SDL_GLContext &context);

public:
    /**
     * Constructor for GLMain.
     * @param window SDL window in which the rendered images are drawn.
     * @param context OpenGL context for which is related to the window.
     * @param objFilePaths Paths to the .obj files which should be loaded.
     * @param materialFolder Path to the folder in which the material files are stored.
     */
    GLMain(SDL_Window *window, SDL_GLContext &context, std::vector<std::string> &objFilePaths, std::string &materialFolder);

    /**
     * Destructor, calls clearOpenGLSceneAndShaderProgram.
     */
    ~GLMain();

    /**
     * Destructs the scene and destructs the shader program.
     */
    void clearOpenGLSceneAndShaderProgram();

    /**
     * Initializes new labyrinth.
     * @param objLabyrinthPath Path to the .obj file in which the new labyrinth is stored.
     * @param materialFolder Path to the folder in which the material files are stored.
     */
    void initializeNewLabyrinth(std::string &objLabyrinthPath, std::string &materialFolder);

    /**
     * Function called once in the graphics loop to draw the scene.
     */
    void display();

    /**
     * Function changes the window size used for rendering according to the parameters.
     * @param width new window width
     * @param height new window height
     */
    void reshape(int width, int height);

    /**
     * Function rotates the model with specific id around axis for angle degrees.
     * @param id Model id in the model vector of scene.
     * @param axis 0 -> x-axis, 1 -> z-axis.
     * @param angle Angle of rotation in degrees.
     */
    void rotateModelAroundAxis(int id, int axis, float angle);

    /**
     * Function resets the rotation matrix around an axis of model with specific id to identity.
     * @param id Model id in the model vector of scene.
     */
    void resetModelRotationAroundAxis(int id);

    /**
     * Get the scene object.
     * @return Scene object.
     */
    const std::shared_ptr<Scene> &getScene() const;
};

