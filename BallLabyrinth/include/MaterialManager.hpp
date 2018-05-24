#pragma once

#include <string>
#include <memory>
#include <glm/vec3.hpp>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif
#include <iostream>
#include "ShaderProgram.hpp"

struct Material {
public:
    struct Color {
    public:
        Color(glm::vec3 rgb = glm::vec3(0, 0, 0)) : rgb(rgb) {};

        Color(float r, float g, float b) : rgb(glm::vec3(r, g, b)) {};

        glm::vec3 rgb;
    };

    Material(const std::string &name, const Color diffuseColor, const Color specularColor, GLfloat shininess,
             const std::shared_ptr<ShaderProgram> shader = nullptr) : name(name), diffuseColor(diffuseColor),
                                                            specularColor(specularColor), shininess(shininess),
                                                            shader(shader) {
    }

    std::string name;
    Color diffuseColor;
    Color specularColor;
    GLfloat shininess;
    const std::shared_ptr<ShaderProgram> shader;
};


class MaterialManager {
public:
    static MaterialManager &getInstance();

    void addMaterial(std::shared_ptr<Material> material);

    const std::shared_ptr<Material> getById(int id) const;
    const std::shared_ptr<Material> getByName(const std::string &name) const;

private:
    std::vector<std::shared_ptr<Material>> materials;

    MaterialManager();
    MaterialManager(const MaterialManager &other) = delete;
    MaterialManager &operator=(const MaterialManager &other) = delete;
};



