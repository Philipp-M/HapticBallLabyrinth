//
// Created by steve on 1/10/17.
//

#pragma once

#include <glm/vec3.hpp>
#include "MaterialManager.hpp"

class PointLight {
public:
    PointLight(const glm::vec3 &position, const Material::Color &color = Material::Color(glm::vec3(1.0)),
               float intensity = 1.0, float attenuation = 0.5f, float ambient = 0.1f);

    const glm::vec3 &getPosition() const;

    const Material::Color &getColor() const;

    float getAmbient() const;

    float getAttenuation() const;

    void addToShader(ShaderProgram &shaderProgram, GLuint num);

private:
    glm::vec3 position;
    Material::Color color;
    float intensity;
    float ambient;
    float attenuation;
};


