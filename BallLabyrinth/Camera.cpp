//
// Created by steve on 1/10/17.
//

#include "Camera.hpp"

Camera::Camera(float height, float width, float fieldOfView, float nearPlane, float farPlane, glm::vec3 initPos,
               glm::vec3 initUp, float yaw, float pitch) : height(height), width(width), fieldOfView(fieldOfView),
 nearPlane(nearPlane), farPlane(farPlane), position(initPos), up(initUp), yaw(yaw), pitch(pitch) {
    updateCameraParameters();
}

void Camera::rotate(float pitch, float yaw) {
    this->pitch += pitch;
    this->yaw += yaw;

    //check if pitch out of range
    this->pitch = std::max(-90.f, std::min(90.0f, this->pitch));
    updateCameraParameters();
}

void Camera::move(float x, float y, float z) {
    position = x * front + y * right + z * up;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fieldOfView), width / height, nearPlane, farPlane);
}

const glm::vec3 &Camera::getPosition() const {
    return position;
}

void Camera::updateCameraParameters() {
        glm::vec3 direction;

        direction.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
        direction.y = std::sin(glm::radians(pitch));
        direction.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));

        front = glm::normalize(direction);
        right = glm::normalize(glm::cross(front, up));
    }
