#include "Physics.hpp"

#include <glm/gtx/string_cast.hpp>


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
    inverseInertiaTensor = glm::mat3(1.0 / (2.0 / 5.0 * mass * radius * radius));
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
//    float j = -(collisionEpsilon + 1) * glm::dot(velocity, collision.collisionNormal);
//    j /= glm::dot(collision.collisionNormal, collision.collisionNormal) * (1.0 / mass);
//    velocity += j * collision.collisionNormal / mass;

    auto rBall = -collision.collisionNormal * radius;

    float numerator = -(collisionEpsilon + 1.0f) * glm::dot(velocity, collision.collisionNormal);
    float denominator = (1.0f / mass) * glm::dot(collision.collisionNormal, collision.collisionNormal)
                     + glm::dot(glm::cross(glm::cross(inverseInertiaTensor * rBall, collision.collisionNormal), rBall), collision.collisionNormal);

    float j = numerator/denominator;
    velocity += j * collision.collisionNormal / mass;
    omega += inverseInertiaTensor * glm::cross(rBall, (j * collision.collisionNormal));
//    velocity = glm::reflect(velocity, collision.collisionNormal*0.8f);
}

void Physics::Ball::updatePhysics(float dt, glm::vec3 earthAcceleration) {
    // Total force calculation
    force = earthAcceleration * mass;

    // Total torque calculation

    // Update velocity
    velocity += dt * force / mass;

    // Update position and linear velocity
    centerpoint += dt * velocity;

    // Update rotation matrix
    rotation += dt * glm::matrixCross3(omega) * rotation;
    rotation = glm::orthonormalize(rotation);

    // Update angular momentum
    angularMomentum += dt * torque;

    // Update inverse inertia tensor
    inverseInertiaTensor = rotation * inverseInertiaTensor * glm::transpose(rotation);

    // Update angular velocity
    omega = inverseInertiaTensor * angularMomentum;

    velocity *= 1.0 - dt / 1.0;

    force.x = 0.0;
    force.y = 0.0;
    force.z = 0.0;
}

void Physics::Ball::updateGraphicsModel() {

    // Calculate centerpoint for graphics model
    glm::vec3 graphicCenter = centerpoint;
    float tmp = graphicCenter.z;
    graphicCenter.z = graphicCenter.y;
    graphicCenter.y = tmp;

    // Update translation matrix of graphicsModel
    graphicsModel->resetTranslationMatrix();
    graphicsModel->translate(graphicCenter);

//    // Calculate rotation of graphicsModel
//    glm::mat4 graphicsModelRotation(rotation);
//    graphicsModelRotation *= glm::mat4(1.0, 0.0, 0.0, 0.0,
//                                       0.0, 0.0, 1.0, 0.0,
//                                       0.0, 1.0, 0.0, 0.0,
//                                       0.0, 0.0, 0.0, 1.0);
//
//    // Update rotation matrix around origin of graphicsModel
//    graphicsModel->resetRotationMatrixModelOrigin();
//    graphicsModel->rotateAroundModelOrigin(graphicsModelRotation);
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
            } else if (linecount == 2) {
                in >> startx >> starty >> widthx >> widthy;
            } else {
                float x1, y1, x2, y2;
                in >> x1 >> y1 >> x2 >> y2;
                walls.push_back(StaticObject(x1, y1, x2, y2, floorheight, wallheight, wallwidth));
            }
        }
        myfile.close();

        walls.push_back(StaticObject(startx, starty, startx + widthx, starty, 0.0, floorheight, widthy));
        std::cout << "walls loaded: " << walls.size() << std::endl;
    }
}

void Physics::rotateEarthAccelerationXY(float pitch, float yaw) {
    lock.lock();
    this->pitch = pitch;
    earthAcceleration = glm::rotateX(glm::vec3(0.0, 0.0, -EARTH_ACCEL), glm::radians(pitch));
    this->yaw = yaw;
    earthAcceleration = glm::rotateY(earthAcceleration, glm::radians(yaw));
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

    while (!quit) {
        std::this_thread::sleep_for(dt);
        dtElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - elapseTime).count() / 1000.0f;
        elapseTime = std::chrono::high_resolution_clock::now();
        handleCollisions();
        lock.lock();
        ballObjects[0].updatePhysics(dtElapsed, earthAcceleration);
//        ballObjects[0].updateGraphicsModel();
        lock.unlock();
    }
}

void Physics::quitPhysics() {
    lock.lock();
    quit = true;
    lock.unlock();
}

void Physics::updateGraphicsModel() {
    lock.lock();
    ballObjects[0].updateGraphicsModel();
    lock.unlock();
}

