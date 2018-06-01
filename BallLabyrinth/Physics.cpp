#define GLM_ENABLE_EXPERIMENTAL

#include "Physics.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp>


Physics::StaticObject::StaticObject(float x1, float y1, float x2, float y2, float floorheight, float wallheight,
                                    float wallwidth) {
    if (y1 == y2) {
        if (x1 > x2) {
            edgepointMin.x = x2;
            edgepointMax.x = x1 + wallwidth;
        } else {
            edgepointMin.x = x1;
            edgepointMax.x = x2 + wallwidth;
        }

        edgepointMin.y = y1;
        edgepointMax.y = y1 + wallwidth;
    } else if (x1 == x2) {
        if (y1 > y2) {
            edgepointMin.y = y2;
            edgepointMax.y = y1 + wallwidth;
        } else {
            edgepointMin.y = y1;
            edgepointMax.y = y2 + wallwidth;
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
    auto rBall = -collision.collisionNormal * radius;

    float numerator = -(collisionEpsilon + 1.0f) * glm::dot(velocity, collision.collisionNormal);
    float denominator = (1.0f / mass) * glm::dot(collision.collisionNormal, collision.collisionNormal)
                        +
                        glm::dot(glm::cross(glm::cross(inverseInertiaTensor * rBall, collision.collisionNormal), rBall),
                                 collision.collisionNormal);

    float j = numerator / denominator;
    velocity += j * collision.collisionNormal / mass;
    omega += inverseInertiaTensor * glm::cross(rBall, (j * collision.collisionNormal));
}

void Physics::Ball::updatePhysics(float dt, glm::vec3 earthAcceleration) {

    // calculate rolling resistance
    float forceRoll = rollingFrictionCoefficient * std::abs(earthAcceleration.z) * mass;

    // Total force calculation
    force = earthAcceleration * mass;

    if((force.x * force.x + force.y * force.y) > (forceRoll*forceRoll) || glm::l2Norm(velocity) > 0.01f)
        force += -glm::normalize(velocity) * forceRoll; // friction term

    // Total torque calculation
//    if(force.x != 0.0 || force.y != 0.0) {
//        float angle = glm::dot(earthAcceleration, glm::vec3(1.0f, 0.0f, 0.0f)) / (std::sqrt(
//                earthAcceleration.x * earthAcceleration.x + earthAcceleration.y * earthAcceleration.y +
//                earthAcceleration.z * earthAcceleration.z));
//        angle = std::acos(angle);
//        torque = (2.0f / 7.0f * mass * radius * std::sin(angle)) * earthAcceleration;
//    } else {
//        torque = glm::vec3(0.0, 0.0, 0.0);
//    }

    // Update velocity
    velocity += dt * force * 0.7f / mass; // 0.7 factor from the solid sphere inertial tensor
    //velocity += dt * velocity * earthAcceleration.z * 0.002f; // friction term

    // Update position and linear velocity
    centerpoint += dt * velocity;

//    std::cout << "pos: " << glm::to_string(centerpoint) << std::endl;

    // Update rotation matrix
    rotation += dt * glm::matrixCross3(omega) * rotation;
    rotation = glm::orthonormalize(rotation);

    // Update angular momentum
    angularMomentum += dt * torque;

    // Update inverse inertia tensor
    inverseInertiaTensor = rotation * inverseInertiaTensor * glm::transpose(rotation);

    // Update angular velocity
    omega = inverseInertiaTensor * angularMomentum;
//    if (velocity.x < -0.0001 || velocity.x > 0.0001 || velocity.y < -0.0001 || velocity.y > 0.0001) {
//        omega = velocity/radius;
//    } else {
//        omega.x = 0.0,
//        omega.y = 0.0;
//        omega.z = 0.0;
//    }

    // Simple calculations to let the ball roll, is just used for update of visual model. It is not used for the physics.
    if (velocity.x < -0.0001 || velocity.x > 0.0001 || velocity.y < -0.0001 || velocity.y > 0.0001) {
        // omegaSimple = velocity / radius
        glm::vec3 omegaSimple = velocity / radius;
        omegaSimple.z = omegaSimple.y;
        omegaSimple.y = 0.0;
        // Normal of vector along y axis and velocity direction -> axis around which the ball is rotating while rolling.
        rotationAxisSimple = glm::cross(glm::vec3(0.0, 1.0, 0.0), glm::normalize(omegaSimple));
        // Calculating delta angle, which the ball has to rotate.
        omegaSimple *= dt;
        rotationAngleSimple = std::sqrt(omegaSimple.x * omegaSimple.x + omegaSimple.y * omegaSimple.y + omegaSimple.z * omegaSimple.z);
        rotationMatGraphicsModel = glm::rotate(glm::mat4(1.0), rotationAngleSimple, rotationAxisSimple) * rotationMatGraphicsModel;
    } else {
        rotationAngleSimple = 0.0;
    }

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
//    // Update rotation matrix around origin of graphicsModel
//    graphicsModel->resetRotationMatrixModelOrigin();
//    graphicsModel->rotateAroundModelOrigin(graphicsModelRotation);

    // Update rotation of graphics model according to simple calculation of rotation depending on velocity
    if (velocity.x < -0.0001 || velocity.x > 0.0001 || velocity.y < -0.0001 || velocity.y > 0.0001) {
        glm::mat4 graphicsModelRotation = glm::rotate(glm::mat4(1.0), rotationAngleSimple, rotationAxisSimple);
        graphicsModel->resetRotationMatrixModelOrigin();
        graphicsModel->rotateAroundModelOrigin(rotationMatGraphicsModel);
    }
}

Physics::Physics(float dt) : dt(dt), quit(false), earthAcceleration(0.0, 0.0, -EARTH_ACCEL), pitch(0.0),
                             yaw(0.0) {
}

void Physics::addBall(std::shared_ptr<GraphicsModel> model, float mass, float radius, float collisionEpsilon, float rollingFriction) {
    ballObjects.emplace_back(Ball(model, mass, radius, collisionEpsilon, rollingFriction));
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
                walls.emplace_back(x1, y1, x2, y2, floorheight, wallheight, wallwidth);
            }
        }
        myfile.close();

        walls.emplace_back(StaticObject(startx, starty, startx + widthx, starty, 0.0, floorheight, widthy));
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

bool Physics::inGame() const {
    lock.lock();
    glm::vec3 tmp = ballObjects[0].centerpoint;
    lock.unlock();
    if(tmp.x > 15.0 || tmp.x < -15.0 || tmp.y > 15.0 || tmp.y < -15.0) {
        if(tmp.z < 0.0) {
            return false;
        }
    }
    return true;
}

void Physics::updateGraphicsModel() {
    lock.lock();
    ballObjects[0].updateGraphicsModel();
    lock.unlock();
}


