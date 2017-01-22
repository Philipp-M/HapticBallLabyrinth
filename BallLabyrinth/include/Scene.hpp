//
// Created by steve on 1/5/17.
//

#pragma once


#include <memory>
#include <vector>

#include "GraphicsModel.hpp"
#include "PointLight.hpp"
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "Shader.hpp"

/**
 * Class representing the whole graphics scene.
 */
class Scene {
private:
    std::vector<std::shared_ptr<GraphicsModel>> models; /**< Container to store all models. */
    std::vector<std::shared_ptr<PointLight>> lights; /**< Container to store all lights. */
    Camera camera; /**< Camera of the scene */

public:
    /**
     * Construct scene and loads models and materials and sets the camera.
     * @param objFilePaths Paths to .obj files which contain the necessary models.
     * @param materialRefFolder Path to the folder containing the .mtl material files.
     */
    Scene(std::vector<std::string> &objFilePaths, std::string &materialRefFolder);

    /**
     * Loads a new .obj file and adds all the models to the scene.
     * @param objFilePath Path to the .obj file.
     * @param materialRefFolder Path to the folder containing the .mtl material files.
     */
    void loadObjFile(std::string objFilePath, std::string &materialRefFolder);

    /**
     * Draws the whole scene by setting necessary paramerters and calling the draw function from all the models.
     * @param shaderProgram OpenGL program for which the draw call is executed.
     */
    void draw(ShaderProgram &shaderProgram);

    /**
     * Rotate model around global axis.
     * @param id Model id
     * @param axis Axis around the model is rotated: x - 0, y - 1, z - 2.
     * @param angle Rotation angle.
     */
    void rotateModelAroundAxis(int id, int axis, float angle);

    /**
     * Reset rotation of model around global axis.
     * @param id Model id.
     */
    void resetModelRotationAroundAxis(int id);

    /**
     * Get model by name.
     * @param name Model name.
     * @return Pointer to model.
     */
    std::shared_ptr<GraphicsModel> getModelByName(std::string name);
};



