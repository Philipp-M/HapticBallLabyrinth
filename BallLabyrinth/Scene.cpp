//
// Created by steve on 1/5/17.
//

#include <tiny_obj_loader.hpp>
#include <iostream>
#include "Scene.hpp"

Scene::Scene(std::string &objFilePath, std::string &materialRefFolder) :
        camera(600, 800, 60.0, 0.01, 1000.0, glm::vec3(0.0, 40.0, 0.0), glm::vec3(0.0, 0.0, -1.0), 0.0, -90.0) {
    loadObjFile(objFilePath, materialRefFolder);

    lights.push_back(std::shared_ptr<PointLight>(new PointLight(glm::vec3(10.0, 30.0, 10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 1.0, 1.0)));
    lights.push_back(std::shared_ptr<PointLight>(new PointLight(glm::vec3(-10.0, 30.0, -10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 1.0, 1.0)));
    lights.push_back(std::shared_ptr<PointLight>(new PointLight(glm::vec3(10.0, 30.0, -10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 1.0, 1.0)));
    lights.push_back(std::shared_ptr<PointLight>(new PointLight(glm::vec3(-10.0, 30.0, 10.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 150, 1.0, 1.0)));
    lights.push_back(std::shared_ptr<PointLight>(new PointLight(glm::vec3(0.0, 60.0, 0.0), Material::Color(glm::vec3(1.0, 1.0, 1.0)), 1500, 1.0, 1.0)));
}

void Scene::loadObjFile(std::string filename, std::string &materialRefFolder) {
    std::vector<tinyobj::shape_t> objectShape;
    std::vector<tinyobj::material_t> objectMaterial;

    tinyobj::LoadObj(objectShape, objectMaterial, filename.c_str(), materialRefFolder.c_str());

    for (auto &m: objectMaterial) {
        std::cout << m.name << " diffuse: " << m.diffuse[0] << "/" << m.diffuse[1] << "/" << m.diffuse[2] << std::endl;
        MaterialManager::getInstance().addMaterial(std::shared_ptr<Material>(
                new Material(m.name, Material::Color(m.diffuse[0], m.diffuse[1], m.diffuse[3]),
                             Material::Color(m.specular[0], m.specular[1], m.specular[2]), m.shininess)));
    }

    for (auto &object: objectShape) {
        if (object.mesh.material_ids.size() > 0) {
            models.push_back(std::shared_ptr<GraphicsModel>(new GraphicsModel(object.mesh, object.name,
                                                              MaterialManager::getInstance().getByName(
                                                                      objectMaterial[object.mesh.material_ids[0]].name))));
        } else {
            models.push_back(std::shared_ptr<GraphicsModel>(new GraphicsModel(object.mesh, object.name)));
        }
//        if (object.name == "light") {
//            std::shared_ptr<GraphicsModel> m = models[models.size() - 1];
//            lights.push_back(std::shared_ptr<PointLight>(
//                    new PointLight(m->getCentroid(), m->getMaterial()->diffuseColor, 0.5, 0.001)));
//        }
    }
}

void Scene::draw(ShaderProgram &shaderProgram) {
    //shaderProgram.bind();
    shaderProgram.setUniform3f("cameraPosition", camera.getPosition());
    shaderProgram.setUniform1i("numPointLights", lights.size());

    for(int i=0; i<lights.size(); i++) {
        lights[i]->addToShader(shaderProgram, i);
    }

    for (auto &model: models) {
        shaderProgram.setMatrixUniform4f("MVPMatrix", camera.getProjectionMatrix() * camera.getViewMatrix() * model->getObjectTransformationMatrix());
        model->draw(shaderProgram);
    }

    //shaderProgram.unbind();
}

void Scene::rotateModel(int id, int axis, float angle) {

    if(id < models.size() && id >= 0) {
        if(axis == 0) {
            models[id]->rotateAroundX(angle);
        } else if(axis == 1) {
            models[id]->rotateAroundZ(angle);
        } else {
            std::cerr << "Wrong parameter for axis -> nothing happened." << std::endl;
        }
    } else {
        std::cerr << "Wrong id, model does not exist." << std::endl;
    }
}


