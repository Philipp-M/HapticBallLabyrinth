#pragma once

#include <mutex>
#include <iostream>
#include <glm/glm.hpp>
#include "HandleInterface.hpp"

template<typename T>
int
sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

class HapticForceManager
{
private:
    mutable std::recursive_mutex mutex;
    glm::vec2                    ballCollisionForce;
    glm::vec2                    wallPos;
    float                        wallK;
    float                        centerSpringK;
    float                        centerSpringDead;
    HandleInterface&             handleInterface;

    glm::vec2 getCenterSpringForce()
    {
        auto  pos1 = handleInterface.getPos1();
        auto  pos2 = handleInterface.getPos2();
        float force1, force2;

        if (std::abs(pos1) > centerSpringDead)
            force1 = (pos1 - sgn(pos1) * centerSpringDead) * centerSpringK;
        else
            force1 = 0.0f;

        if (std::abs(pos2) > centerSpringDead)
            force2 = (pos2 - sgn(pos2) * centerSpringDead) * centerSpringK;
        else
            force2 = 0.0f;

        return glm::vec2(force1, force2);
    }

    glm::vec2 getWallForce()
    {
        auto      pos1 = handleInterface.getPos1();
        auto      pos2 = handleInterface.getPos2();
        glm::vec2 force(0, 0);
        if (pos1 > wallPos.y)
            force.x = wallK * (pos1 - wallPos.y);
        else if (pos1 < wallPos.x)
            force.x = wallK * (pos1 - wallPos.x);
        if (pos2 > wallPos.y)
            force.y = wallK * (pos2 - wallPos.y);
        else if (pos2 < wallPos.x)
            force.y = wallK * (pos2 - wallPos.x);
        return force;
    }

public:
    bool enableCenterSpring;
    bool enableBallCollision;
    bool enableWalls;

    HapticForceManager(HandleInterface& handleInterface,
                       glm::vec2        wallPos          = glm::vec2(-40.0f, 40.0f),
                       float            wallK            = 0.5f,
                       float            centerSpringK    = 0.01f,
                       float            centerSpringDead = 10.0f)
    : handleInterface(handleInterface)
    , wallPos(wallPos)
    , wallK(wallK)
    , centerSpringK(centerSpringK)
    , centerSpringDead(centerSpringDead)
    , enableCenterSpring(true)
    , enableBallCollision(true)
    , enableWalls(true)
    {
    }

    void setBallCollisionForce(const glm::vec2& force)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        ballCollisionForce = force;
    }

    glm::vec2 getHandleForce()
    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        glm::vec2                             force(0, 0);
        if (enableBallCollision)
            force += ballCollisionForce;
        if (enableCenterSpring)
            force += getCenterSpringForce();
        if (enableWalls)
            force += getWallForce();
        return force;
    }
};
