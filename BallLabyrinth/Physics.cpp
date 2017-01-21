//
// Created by steve on 1/18/17.
//

#include "Physics.hpp"


Physics::StaticObject::StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight,
                                    float wallwidth) {
    if (y1 == y2) {
        if (x1 > x2) {
            edgepointMin.x = x2;
            edgepointMax.x = x1;
        } else {
            edgepointMin.x = x1;
            edgepointMax.x = x2;
        }

        edgepointMin.y = y1;
        edgepointMax.y = y1 + wallwidth;
    } else if (x1 == x2) {
        if (y1 > y2) {
            edgepointMin.y = y2;
            edgepointMax.y = y1;
        } else {
            edgepointMin.y = y1;
            edgepointMax.y = y2;
        }

        edgepointMin.x = x1;
        edgepointMax.x = x1 + wallwidth;
    }
    edgepointMin.z = floorheight;
    edgepointMax.z = floorheight + wallheight;
}

void Physics::Ball::calculateInverseInertiaTensor() {
    inverseInertiaTensor = glm::mat3(1 / (2 / 5 * mass * radius * radius));
}

Physics::Collision Physics::Ball::collisionCheck(const Physics::StaticObject &wall) {
    Collision collision;
    float x = std::max(wall.edgepointMin.x, std::min(centerpoint.x, wall.edgepointMax.x));
    float y = std::max(wall.edgepointMin.y, std::min(centerpoint.y, wall.edgepointMax.y));
    float z = std::max(wall.edgepointMin.z, std::min(centerpoint.z, wall.edgepointMax.z));

    // distance between point and boundingbox
    collision.distance = (centerpoint.x - x) * (centerpoint.x - x) +
                         (centerpoint.y - y) * (centerpoint.y - y) +
                         (centerpoint.z - z) * (centerpoint.z - z);
    if (collision.distance <= radius * radius) {
        collision.collision = true;
        collision.distance = std::sqrt(collision.distance);
        collision.collisionNormal = glm::normalize(glm::vec3(centerpoint.x - x, centerpoint.y - y, centerpoint.z - z));
    }

    return collision;
}

void Physics::Ball::resetPosition(Physics::Collision &collision) {
    centerpoint += (radius - collision.distance) * collision.collisionNormal;
}

void Physics::Ball::updateCollisionImpulse(Physics::Collision &collision) {
//    glm::vec3 rBall = -collision.collisionNormal * radius;
//
//    float j = (-(collisionEpsilon + 1) * glm::dot(velocity, collision.collisionNormal))
//              / ((1.0 / mass) * (glm::dot(collision.collisionNormal, collision.collisionNormal)
//                                 + glm::dot(
//            glm::cross(glm::cross(inverseInertiaTensor * rBall, collision.collisionNormal), rBall),
//            collision.collisionNormal)));
//    velocity += j * collision.collisionNormal / mass;
//    omega += inverseInertiaTensor * glm::cross(rBall, (j * collision.collisionNormal));

    float j = -(collisionEpsilon+1)*glm::dot(velocity,collision.collisionNormal);
          j /= glm::dot(collision.collisionNormal,collision.collisionNormal)*(1.0/mass);
    velocity += j * collision.collisionNormal / mass;
//    velocity = glm::reflect(velocity, collision.collisionNormal*0.8f);
}

void Physics::Ball::updatePhysics(float dt, glm::vec3 earthAcceleration) {
    // Total force calculation
    force = earthAcceleration * mass;

    // Total torque calculation

    // Update position and linear velocity
    oldCenterpoint = centerpoint;
    centerpoint += dt * velocity;

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

    velocity *= 1.0 - dt/1.0;

    force.x = 0.0;
    force.y = 0.0;
    force.z = 0.0;
}

void Physics::Ball::updateGraphicsModel(float pitch, float yaw) {
    glm::vec3 transVec = centerpoint;
    glm::vec3 helperVec = oldCenterpoint;

    glm::rotateX(helperVec, glm::radians(pitch));
    glm::rotateY(helperVec, glm::radians(yaw));

    glm::rotateX(transVec, glm::radians(pitch));
    glm::rotateY(transVec, glm::radians(yaw));

    transVec -= helperVec;

    // Calculate centerpoint for graphics model
    float tmp = transVec.z;
    transVec.z = transVec.y;
    transVec.y = tmp;

    graphicsModel->translate(transVec);
}

Physics::Physics(float dt) : dt(dt), quit(false), earthAcceleration(0.0, 0.0, -EARTH_ACCEL), pitch(0.0),
                                         yaw(0.0) {
}

void Physics::addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius, float collisionEpsilon) {
    ballObjects.push_back(Ball(model, mass, radius, collisionEpsilon));
}

void Physics::addWalls(std::string file) {
    std::string line;
    int linecount = 0;
    std::ifstream myfile(file);
    if (myfile.is_open()) {
        float wallheight, floorheight, wallwidth;
        float startx, starty, widthx, widthy;
        for (std::string line; std::getline(myfile, line);) {  //read stream line by line
            linecount++;
            std::istringstream in(line);
            if (linecount == 1) {
                in >> wallheight >> floorheight >> wallwidth; //h f t
            } else if(linecount == 2) {
                in >> startx >> starty >> widthx >> widthy;
            } else {
                float x1, y1, x2, y2;
                in >> x1 >> y1 >> x2 >> y2;
                walls.push_back(StaticObject(x1, y1, x2, y2, floorheight, wallheight, wallwidth));
            }
        }
        myfile.close();

        walls.push_back(StaticObject(startx, starty, startx+widthx, starty, 0.0, floorheight, widthy));
        std::cout << "walls loaded: " << walls.size() << std::endl;
    }
}

void Physics::rotateEarthAccelerationX(float pitch) {
    lock.lock();
    this->pitch += pitch;
    earthAcceleration = glm::rotateX(earthAcceleration, glm::radians(-pitch));
    lock.unlock();
}

void Physics::rotateEarthAccelerationY(float yaw) {
    lock.lock();
    this->yaw += yaw;
    earthAcceleration = glm::rotateY(earthAcceleration, glm::radians(-yaw));
    lock.unlock();
}

void Physics::handleCollisions() {
    Collision collision;
    Ball ball = ballObjects[0];
    for (auto &wall: walls) {
        collision = ball.collisionCheck(wall);
        if (collision.collision) {
            ballObjects[0].resetPosition(collision);
            ballObjects[0].updateCollisionImpulse(collision);
        }
    }

}

void Physics::update() {
    auto elapseTime = std::chrono::high_resolution_clock::now();

    while(!quit) {
        std::this_thread::sleep_for(dt);
        dtElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - elapseTime).count() / 1000.0f;
        elapseTime = std::chrono::high_resolution_clock::now();
        handleCollisions();
        lock.lock();
        ballObjects[0].updatePhysics(dtElapsed, earthAcceleration);
        ballObjects[0].updateGraphicsModel(pitch, yaw);
        lock.unlock();
    }
}

void Physics::quitPhysics() {
    lock.lock();
    quit = true;
    lock.unlock();
}

