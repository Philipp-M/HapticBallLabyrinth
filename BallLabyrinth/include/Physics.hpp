//
// Created by steve on 1/18/17.
//

#pragma once

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include "GraphicsModel.hpp"

#define EARTH_ACCEL 9.81

class Physics {
public:
    struct StaticObject {

    };
    struct Ball {
        std::shared_ptr<GraphicsModel> graphicsModel;

        float mass;
        float radius;
        glm::vec3 centerpoint;

        glm::vec3 velocity;
        glm::vec3 angularMomentum;
        glm::vec3 omega;

        glm::mat3 rotation;
        glm::mat3 inverseInertiaTensor;

        glm::vec3 force;

        Ball(std::shared_ptr<GraphicsModel> &model, float mass=0.01, float radius=1.0):
                graphicsModel(model), mass(mass), radius(radius), velocity(0.0), angularMomentum(0.0), omega(0.0), rotation(1.0) {
            centerpoint = model->getCentroid();
            force = glm::vec3(0.0, 0.0, -EARTH_ACCEL * mass);
            inverseInertiaTensor = calculateInverseInertiaTensor(radius, mass);
        }


        glm::mat3 calculateInverseInertiaTensor(float radius, float mass) {
            return glm::mat3();
        }
    };
private:
    double time;
    double dt;
    std::vector<Ball> ballObjects;
    std::vector<StaticObject> walls;

public:
    Physics(double time = 0.0, double dt = 0.001);


};

