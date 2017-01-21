//
// Created by steve on 1/18/17.
//

#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>

#define GLM_FORCE_RADIANS

#include <glm/vec3.hpp>
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "GraphicsModel.hpp"

#define EARTH_ACCEL 9810

static std::mutex lock;

class Physics {
public:
    struct Collision {
        bool collision = false;
        glm::vec3 collisionNormal;
        float distance;
    };
    //AABB box according to: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
    struct StaticObject {
        glm::vec3 edgepointMin;
        glm::vec3 edgepointMax;

        StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight, float wallwidth);
    };

    struct Ball {
        std::shared_ptr<GraphicsModel> graphicsModel;

        float mass;
        float radius;
        float collisionEpsilon;
        glm::vec3 centerpoint;
        glm::vec3 oldCenterpoint;

        glm::vec3 velocity;
        glm::vec3 angularMomentum;
        glm::vec3 omega;

        glm::mat3 rotation;
        glm::mat3 inverseInertiaTensor;
        glm::vec3 torque;

        glm::vec3 force;

        Ball(std::shared_ptr<GraphicsModel> &model, float mass, float radius, float collisionEpsilon) :
                graphicsModel(model), mass(mass), radius(radius), collisionEpsilon(collisionEpsilon), centerpoint(model->getCentroid().x, model->getCentroid().z, model->getCentroid().y), velocity(0.0),
                angularMomentum(0.0), omega(0.0), rotation(1.0), torque(0.0), force(0.0, 0.0, 0.0) {
            calculateInverseInertiaTensor();
        }

        void calculateInverseInertiaTensor();

        Collision collisionCheck(const StaticObject &wall);

        void resetPosition(Collision &collision);

        void updateCollisionImpulse(Collision &collision);

        void updatePhysics(float dt, glm::vec3 earthAcceleration);

        void updateGraphicsModel(float pitch, float yaw);
    };


private:
    std::chrono::duration<float> dt;
    float dtElapsed;
    bool quit;
    glm::vec3 earthAcceleration;
    float pitch, yaw;
    std::vector<Ball> ballObjects;
    std::vector<StaticObject> walls;

public:
    Physics(float dt = 0.001);

    void addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius, float collisionEpsilon);

    void addWalls(std::string file);

    void rotateEarthAccelerationX(float pitch);
    void rotateEarthAccelerationY(float yaw);

    void handleCollisions();

    void update();

    void quitPhysics();
};


