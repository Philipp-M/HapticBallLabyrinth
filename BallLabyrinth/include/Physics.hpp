//
// Created by steve on 1/18/17.
//

#pragma once

#include <vector>

#define GLM_FORCE_RADIANS

#include <glm/vec3.hpp>
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include "GraphicsModel.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define EARTH_ACCEL 9.81

class Physics {
public:
    struct StaticObject {
        glm::vec3 edgepoint1, edgepoint2;
        glm::vec3 normals;

        StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight, float wallwidth) {
            edgepoint1.x = x1;
            edgepoint1.y = y1;
            edgepoint1.z = floorheight;

            if (y1 == y2) {
                edgepoint2.x = x2;
                edgepoint2.y = y2 + wallwidth;
            } else if (x1 == x2) {
                edgepoint2.x = x2 + wallwidth;
                edgepoint2.y = y2;
            }
            edgepoint2.z = floorheight + wallheight;
        }
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
        glm::vec3 torque;

        glm::vec3 force;

        Ball(std::shared_ptr<GraphicsModel> &model, float mass, float radius) :
                graphicsModel(model), mass(mass), radius(radius), centerpoint(model->getCentroid()), velocity(0.0),
                angularMomentum(0.0), omega(0.0), rotation(1.0), torque(0.0), force(0.0, 0.0, -EARTH_ACCEL * mass) {
            calculateInverseInertiaTensor();
        }

        void update(float dt);

        void calculateInverseInertiaTensor();

        void updateCollision(glm::vec3 collisionNormal);

    };

private:
    float time;
    float dt;
    std::vector<Ball> ballObjects;
    std::vector<StaticObject> walls;

    void loadwalls(std::string file) {
        std::string line;
        int linecount = 0;
        std::ifstream myfile(file);
        if (myfile.is_open()) {
            for (std::string line; std::getline(myfile, line);)   //read stream line by line
            {
                float wallheight, floorheight, wallwidth;
                linecount++;
                std::istringstream in(line);
                if (linecount == 1) {
                    in >> wallheight >> floorheight >> wallwidth; //h f t
                } else {
                    float x1, y1, x2, y2;
                    in >> x1 >> y1 >> x2 >> y2;
                    walls.push_back(StaticObject(x1, y1, x2, y2, floorheight, wallheight, wallwidth));
                }
            }
            myfile.close();
        }
    }

public:
    Physics(double time = 0.0, double dt = 0.001);

    void addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius);

    void update();
};


