#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = glm::vec3(0.0f,2.0f,5.0f);// cameraPosition;
        this->cameraTarget = cameraTarget;
        //this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
        //TODO - Update the rest of camera parameters
        this->worldUpDirection = cameraUp;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        switch (direction) {
        case MOVE_FORWARD:
            cameraPosition += cameraFrontDirection * speed;
            break;

        case MOVE_BACKWARD:
            cameraPosition -= cameraFrontDirection * speed;
            break;

        case MOVE_RIGHT:
            cameraPosition += cameraRightDirection * speed;
            break;

        case MOVE_LEFT:
            cameraPosition -= cameraRightDirection * speed;
            break;
        }
        this->cameraTarget = this->cameraFrontDirection + this->cameraPosition;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 delta;
        delta.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        delta.y = sin(glm::radians(pitch));
        delta.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraTarget = cameraPosition + glm::normalize(delta);
        cameraFrontDirection = glm::normalize(delta);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, worldUpDirection));
        this->cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
    }
}