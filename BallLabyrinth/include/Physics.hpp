//
// Created by steve on 1/18/17.
//

#pragma once

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include "GraphicsModel.hpp"

class Physics {
public:
    struct StaticObject {

    };
    struct Ball {
        float mass;
        float radius;
        glm::vec3 centerpoint;

        glm::vec3 velocity;
        glm::vec3 angularMomentum;
        glm::vec3 omega;

        glm::mat3 rotation;
        glm::mat3 inverseInertiaTensor;

        glm::vec3 force;

        std::shared_ptr<GraphicsModel> graphicsModel;
    };
private:
    double time;
    double dt;
    std::vector<Ball> ballObjects;
    std::vector<StaticObject> walls;

public:
    Physics(double time = 0.0, double dt = 0.001);

};

