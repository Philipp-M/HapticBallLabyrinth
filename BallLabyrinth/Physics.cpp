//
// Created by steve on 1/18/17.
//

#include "Physics.hpp"

void Physics::Ball::update(float dt) {
    // Total force calculation

    // Total torque calculation

    // Update position and linear velocity
    glm::vec3 tVec = centerpoint;
    centerpoint += dt * velocity;

    // Calculate centerpoint for graphics model
    tVec = centerpoint - tVec;
    float tmp = tVec.z;
    tVec.z = tVec.y;
    tVec.y = tmp;
    tVec *= 1000;
    graphicsModel->translate(glm::vec3(tVec));

//    std::cout << centerpoint.x << "/" << centerpoint.y << "/" << centerpoint.z << std::endl;

    // Update velocity
    velocity += dt * force / mass;

    // Update rotation matrix
    rotation += dt * glm::matrixCross3(omega) * rotation;
    rotation = glm::orthonormalize(rotation);

    // Update angular momentum
    angularMomentum += dt * torque;

    // Update inverse inertia tensor
    inverseInertiaTensor = rotation * inverseInertiaTensor * glm::transpose(rotation);

    // Update angular velocity
    omega = inverseInertiaTensor * angularMomentum;
}

void Physics::Ball::calculateInverseInertiaTensor() {
    inverseInertiaTensor = glm::mat3(1 / (2 / 5 * mass * radius * radius));
}

void Physics::Ball::updateCollision(glm::vec3 collisionNormal) {
    velocity = glm::reflect(velocity, collisionNormal);
}



Physics::Physics(double time, double dt): time(time), dt(dt) {
}

void Physics::update() {
    for(auto &ball: ballObjects) {
        ball.update(dt);
    }
}

void Physics::addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius) {
    ballObjects.push_back(Ball(model, mass, radius));
}




