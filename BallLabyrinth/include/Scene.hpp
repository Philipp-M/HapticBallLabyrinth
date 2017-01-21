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


class Scene {
private:
    std::vector<std::shared_ptr<GraphicsModel>> models;
    std::vector<std::shared_ptr<PointLight>> lights;
    Camera camera;

public:
    Scene(std::vector<std::string> &objFilePaths, std::string &materialRefFolder);

    void loadObjFile(std::string objFilePath, std::string &materialRefFolder);

    void draw(ShaderProgram &shader);

    void rotateModelAroundAxis(int id, int axis, float angle);

    void resetModelRotationAroundAxis(int id);

    std::shared_ptr<GraphicsModel> getModelByName(std::string name);
};



