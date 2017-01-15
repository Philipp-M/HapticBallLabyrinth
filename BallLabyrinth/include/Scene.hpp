//
// Created by steve on 1/5/17.
//

#pragma once


#include <memory>
#include <vector>
#include "Model.hpp"
#include "PointLight.hpp"
#include "Camera.hpp"
#include "ShaderProgram.hpp"
#include "Shader.hpp"


class Scene {
private:
    std::vector<std::shared_ptr<Model>> models;
    std::vector<std::shared_ptr<PointLight>> lights;
    Camera camera;

public:
    Scene(std::string &objFilePath, std::string &materialRefFolder);

    void loadObjFile(std::string objFilePath, std::string &materialRefFolder);

    void draw(ShaderProgram &shader);

    void rotateModel(int id, int axis, float angle);
};



