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
#include <cmath>

#define EARTH_ACCEL 9.81

class Physics {
public:
    struct StaticObject { //AABB box according to: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
        glm::vec3 edgepointmin, edgepointmax;

        StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight, float wallwidth) {

            if (y1 == y2) {
                if(x1>x2) {
                    edgepointmin.x = x2;
                    edgepointmax.x = x1;
                } else {
                    edgepointmin.x = x1;
                    edgepointmax.x = x2;
                }

                edgepointmin.y = y1;
                edgepointmax.y = y1 + wallwidth;
            }
            else if (x1 == x2) {
                if(y1>y2) {
                    edgepointmin.y = y2;
                    edgepointmax.y = y1;
                } else {
                    edgepointmin.y = y1;
                    edgepointmax.y = y2;
                }

                edgepointmin.x = x1;
                edgepointmax.x = x1 + wallwidth;
            }
            edgepointmin.z = floorheight;
            edgepointmax.z = floorheight+wallheight;
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
        int linecount=0;
        std::ifstream myfile (file);
        if (myfile.is_open())
        {
            for(std::string line; std::getline(myfile, line); )   //read stream line by line
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

    float getdistancefromAABB(StaticObject aabb, glm::vec3 point) {
        // get box closest point to sphere center by clamping
        float x = std::max(aabb.edgepointmin.x, std::min(point.x, aabb.edgepointmax.x));
        float y = std::max(aabb.edgepointmin.y, std::min(point.y, aabb.edgepointmax.y));
        float z = std::max(aabb.edgepointmin.z, std::min(point.z, aabb.edgepointmax.z));

        // distance between point and boundingbox
        float distance = std::sqrt((x - point.x) * (x - point.x) +
                                 (y - point.y) * (y - point.y) +
                                 (z - point.z) * (z - point.z));

        return distance;
    }

public:
    Physics(double time = 0.0, double dt = 0.001);

    void addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius);

    void update();
};


