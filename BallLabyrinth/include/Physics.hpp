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

#define EARTH_ACCEL 981.0

/**
 * Lock used to synchronize memory, where the physics and the graphics thread access.
 */
static std::mutex lock;

/**
 * Class handling the whole physics of the game, including collisions.
 */
class Physics {
public:
    /**
     * Struct representing a collision.
     */
    struct Collision {
        bool collision = false; /**< Indicates if collision detected. */
        glm::vec3 collisionNormal; /**< Normal pointing from second object (wall) to first object (ball). */
        float distance; /**< Distance between collision point and sphere center. */
    };

    /**
     * Struct representing a static rigid body object as AABB box.
     * AABB box according to: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
     */
    struct StaticObject {
        glm::vec3 edgepointMin; /**< Minimum point of box in physics coordination system. */
        glm::vec3 edgepointMax; /**< Maximum point of box in physics coordination system. */

        /**
         * Constructor for static AABB box (wall).
         * @param x1 x value of point1 on plane.
         * @param y1 y value of point1 on plane.
         * @param x2 x value of point2 on plane.
         * @param y2 y value of point2 on plane.
         * @param floorheight height level of labyrinth floor.
         * @param wallheight height of labyrinth walls.
         * @param wallwidth width of labyrinth walls, used to generate AABB box.
         */
        StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight, float wallwidth);
    };

    /**
     * Struct representing a ball as rigid body.
     */
    struct Ball {
        std::shared_ptr<GraphicsModel> graphicsModel; /**< Pointer to graphicsModel object, which represents this ball visually. */

        float mass; /**< Mass of ball in kilo gramm. */
        float radius; /**< Radius of ball in millimeters. */
        float collisionEpsilon; /**< Constant to describe collision type, it should be between 0.0 and 1.0. */
        glm::vec3 centerpoint; /**< Centerpoint of ball. */

        glm::vec3 velocity; /**< Velocity for rigid body simulation. */
        glm::vec3 angularMomentum; /**< Angular momentum for rigid body simulation. */
        glm::vec3 omega; /**< Angular velocity for rigid body simulation. */

        float rotationAngleSimple;
        glm::vec3 rotationAxisSimple;

        glm::mat3 rotation; /**< Rotation matrix for rigid body simulation. */
        glm::mat3 inverseInertiaTensor; /**< Inverse of inertia tensor for rigid body simulation. */
        glm::vec3 torque; /**< Torque for rigid body simulation. */

        glm::vec3 force; /**< Force acting on rigid body. */

        /**
         * Constructor for rigid body ball object.
         * @param model Graphical model, which represents the ball visually.
         * @param mass Mass of ball.
         * @param radius Radius of ball.
         * @param collisionEpsilon Constant describing the type of physical collision, should be between 0.0 and 1.0.
         */
        Ball(std::shared_ptr<GraphicsModel> &model, float mass, float radius, float collisionEpsilon) :
                graphicsModel(model), mass(mass), radius(radius), collisionEpsilon(collisionEpsilon), velocity(0.0),
                angularMomentum(0.0), omega(0.0), rotation(1.0), torque(0.0), force(0.0, 0.0, 0.0), rotationAngleSimple(0.0),
                rotationAxisSimple(0.0, 0.0, 0.0) {
            calculateInverseInertiaTensor();

            /** Calculate centerpoint of physical model, depending on the centerpoint of the graphical model. */
            glm::vec4 tmp = model->getModelMatrix() * glm::vec4(model->getCentroid(), 1.0);
            centerpoint.x = tmp.x;
            centerpoint.y = tmp.z;
            centerpoint.z = tmp.y;
        }

        /**
         * Function calculates the initial inverse intertia tensor for spheres.
         */
        void calculateInverseInertiaTensor();

        /**
         * Check if ball collides with wall.
         * @param wall Static AABB box for which it is checked if collision happended.
         * @return Collision object, in which the bool collision is set to true, if collision happened.
         */
        Collision collisionCheck(const StaticObject &wall);

        /**
         * Translates the centerpoint of the ball along the collision normal to the position,
         * where the collision distance is equal to the radius of the ball (only collision in one point).
         * @param collision Object of collision,
         */
        void resetPosition(Collision &collision);

        /**
         * Updates the velocity and the angular velocity of the ball, according to the rigid body collision impulse.
         * @param collision Object of collision.
         */
        void updateCollisionImpulse(Collision &collision);

        /**
         * Calculates one rigid body step using the symplectic Euler.
         * @param dt Delta time of this step.
         * @param earthAcceleration Vector describing the earth acceleration for the current step.
         */
        void updatePhysics(float dt, glm::vec3 earthAcceleration);

        /**
         * Updates the graphic model according to the new calculated positions and rotations in the physics simulation.
         */
        void updateGraphicsModel();
    };


private:
    std::chrono::duration<float> dt; /**< Delta time, in which one physics step should be calculated. */
    float dtElapsed; /**< The actual time, which the previous step took, used for next physics update. */
    bool quit; /**< Used to shutdown physics thread. */
    glm::vec3 earthAcceleration; /**< Vector describing the earth acceleration. */
    float pitch, yaw; /**< Angles describing the rotation of the labyrinth. Instead of rotating the whole mesh, only the earthAcceleration is rotated.*/
    std::vector<Ball> ballObjects; /**< Container holding all ball objects in the scene */
    std::vector<StaticObject> walls; /**< Container holding all walls and static objects as AABB boxes. */

public:
    /**
     * Constructor for game physics.
     * @param dt Delta time in which one physics step should be calculated.
     */
    Physics(float dt = 0.001);

    /**
     * Adds ball to physics scene.
     * @param model Graphical model for this physical ball object.
     * @param mass Mass of ball in kg.
     * @param radius Radius of ball in mm.
     * @param collisionEpsilon Constant describing the type of physical collision, should be between 0.0 and 1.0.
     */
    void addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius, float collisionEpsilon);

    /**
     * Loads file for collision geometries and adds AABB boxes for all walls and the floor of the labyrinth.
     * @param file Path to file in which the collision geometries are indicated.
     */
    void addWalls(std::string file);

    /**
     * Set rotation of earth acceleration vector around x axis and y axis.
     * @param pitch Absolute rotation angle around x axis.
     * @param yaw Absolurt rotation angle around y axis.
     */
    void rotateEarthAccelerationXY(float pitch, float yaw);

    /**
     * Checks if there are collisions and updates the position, the velocity and the angular velocity of the ball object.
     */
    void handleCollisions();

    /**
     * Loop function called by the physics thread.
     * Handles all collisions, updates the physics and let the thread sleep for the physics step time.
     */
    void update();

    /**
     * Sets rotation and translation matrices of the graphics model according to the updated physics.
     */
    void updateGraphicsModel();

    /**
     * Let the physics thread leave the loop of the update function, ends physics calculation.
     */
    void quitPhysics();
};


