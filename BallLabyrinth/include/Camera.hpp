//
// Created by steve on 1/10/17.
//

#pragma once

#include <algorithm>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

class Camera {
public:
    Camera(float height = 600.0, float width = 800.0, float fieldOfView = 45.0, float nearPlane = 0.01,
           float farPlane = 1000.0, glm::vec3 initPos = glm::vec3(0.0, 0.0, 0.0),
           glm::vec3 initUp = glm::vec3(0.0, -1.0, 0.0), float yaw = 0.0, float pitch = 0.0);

    /**
     *
     * @param pitch rotation around x axis
     * @param yaw rotation around y axis
     */
    void rotate(float pitch, float yaw);

    void move(float x, float y, float z);

    glm::mat4 getViewMatrix() const;

    glm::mat4 getProjectionMatrix() const;

    const glm::vec3 &getPosition() const;

private:
    float height;
    float width;
    float fieldOfView;
    float nearPlane;
    float farPlane;

    glm::vec3 position;

    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    float yaw;
    float pitch;

    void updateCameraParameters();
};


