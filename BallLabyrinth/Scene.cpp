//
// Created by steve on 1/5/17.
//
#include <memory>
#include <tiny_obj_loader.hpp>
#include <iostream>
#include "Scene.hpp"

void Scene::loadObjFile(std::string filename, std::string &materialRefFolder) {
    std::vector<tinyobj::shape_t> objectShape;
    std::vector<tinyobj::material_t> objectMaterial;

    tinyobj::LoadObj(objectShape, objectMaterial, filename.c_str(), materialRefFolder.c_str());

    for (auto &m: objectMaterial) {
        if(MaterialManager::getInstance().getByName(m.name) == nullptr) {
            std::cout << m.name << " diffuse: " << m.diffuse[0] << "/" << m.diffuse[1] << "/" << m.diffuse[2]
                      << std::endl;
            MaterialManager::getInstance().addMaterial(std::make_shared<Material>(
                    m.name, Material::Color(m.diffuse[0], m.diffuse[1], m.diffuse[2]),
                                 Material::Color(m.specular[0], m.specular[1], m.specular[2]), m.shininess));
        }
    }

    for (auto &object: objectShape) {
        if (!object.mesh.material_ids.empty()) {
            models.push_back(std::make_shared<GraphicsModel>(object.mesh, object.name,
                                                                              MaterialManager::getInstance().getByName(
                                                                                      objectMaterial[object.mesh.material_ids[0]].name)));
        } else {
            models.push_back(std::make_shared<GraphicsModel>(object.mesh, object.name));
        }
    }
}

Scene::Scene(std::vector<std::string> &objFilePaths, std::string &materialRefFolder, int width, int height) :
        camera(height, width, 60.0, 0.01, 1000.0, glm::vec3(0.0, 40.0, 0.0), glm::vec3(0.0, 0.0, -1.0), 0.0, -90.0) {
    for(auto &filePath: objFilePaths) {
        loadObjFile(filePath, materialRefFolder);
    }

    lights.push_back(std::make_shared<PointLight>(glm::vec3(10.0, 60.0, 10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 0.3, 0.2));
    lights.push_back(std::make_shared<PointLight>(glm::vec3(-10.0, 60.0, -10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 0.3, 0.2));
    lights.push_back(std::make_shared<PointLight>(glm::vec3(10.0, 60.0, -10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 0.3, 0.2));
    lights.push_back(std::make_shared<PointLight>(glm::vec3(-10.0, 60.0, 10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 0.3, 0.2));
    lights.push_back(std::make_shared<PointLight>(glm::vec3(0.0, 60.0, 0.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 1500, 0.3, 0.2));
}

void Scene::resetSceneWithNewLabyrinth(std::string &objFilePaths, std::string &materialRefFolder) {
    std::vector<std::shared_ptr<GraphicsModel>> tmp;

    bool modelFound = false;

    for(auto &model: models) {
        if(model->getName() != "Labyrinth") {
            model->resetRotationMatrixModelOrigin();
            model->resetRotationMatrixAxis();
            model->resetTranslationMatrix();
            tmp.push_back(model);
        } else {
            if(!model.unique()) {
                std::cout << "Model found, but is not unique -> could not be deleted." << std::endl;
                exit(-1);
            }
            modelFound = true;
            break;
        }
    }

    if(!modelFound) {
        std::cout << "Could not find model -> shutdown." << std::endl;
        exit(-1);
    }

//    models.clear();
//    models.resize(0);

    models = tmp;
    loadObjFile(objFilePaths, materialRefFolder);

    glm::vec3 initTranslation(-13.0, 2.0, -13.0);
    getModelByName("Ball")->translate(initTranslation);
    glm::vec4 mirrorAxis(1.0, 1.0, -1.0, 1.0);
    getModelByName("Labyrinth")->mirror(mirrorAxis);
}

void Scene::draw(ShaderProgram &shaderProgram) {
    shaderProgram.setUniform3f("cameraPosition", camera.getPosition());
    shaderProgram.setUniform1i("numPointLights", lights.size());

    for(int i = 0; i<lights.size(); i++) {
        lights[i]->addToShader(shaderProgram, i);
    }

    for (auto &model: models) {
        shaderProgram.setMatrixUniform4f("MVPMatrix", camera.getProjectionMatrix() * camera.getViewMatrix() * model->getModelMatrix());
        model->draw(shaderProgram);
    }
}

void Scene::rotateModelAroundAxis(int id, int axis, float angle) {

    if(id < models.size() && id >= 0) {
        if(axis == 0) {
            models[id]->rotateAroundAxisX(angle);
        } else if(axis == 1) {
            models[id]->rotateAroundAxisZ(angle);
        } else {
            std::cerr << "Wrong parameter for axis -> nothing happened." << std::endl;
        }
    } else {
        std::cerr << "Wrong id, model does not exist." << std::endl;
    }
}

void Scene::resetModelRotationAroundAxis(int id) {
    models[id]->resetRotationMatrixAxis();
}

std::shared_ptr<GraphicsModel> Scene::getModelByName(std::string name) {
    for(auto &model: models) {
        if(model->getName() == name) {
            return model;
        }
    }
    return nullptr;
}

void Scene::setWindwoSize(int width, int height) {
    camera.setWindowSize(width, height);
}




