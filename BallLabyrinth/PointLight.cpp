//
// Created by steve on 1/10/17.
//
#include <sstream>
#include <iostream>
#include "PointLight.hpp"

#define SSTR(x) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

PointLight::PointLight(const glm::vec3 &position, const Material::Color &color, float intensity, float attenuation, float ambient) :
        position(position), intensity(intensity), color(color), attenuation(attenuation), ambient(ambient) { }

const glm::vec3 &PointLight::getPosition() const {
    return position;
}

const Material::Color &PointLight::getColor() const {
    return color;
}

float PointLight::getAmbient() const {
    return ambient;
}

float PointLight::getAttenuation() const {
    return attenuation;
}

void PointLight::addToShader(ShaderProgram &shaderProgram, GLuint num) {
    shaderProgram.setUniform3f("allPointLights[" + SSTR(num) + "].position", position);
    shaderProgram.setUniform3f("allPointLights[" + SSTR(num) + "].colorIntensity", intensity * color.rgb);
    shaderProgram.setUniform1f("allPointLights[" + SSTR(num) + "].attenuation", attenuation);
    shaderProgram.setUniform1f("allPointLights[" + SSTR(num) + "].ambient", ambient);
}
