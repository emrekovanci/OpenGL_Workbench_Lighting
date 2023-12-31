#include "Camera.hpp"

Camera::Camera()
{
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == CameraMovement::Forward) { Position += Front * velocity; }
    if (direction == CameraMovement::Backward) { Position -= Front * velocity; }
    if (direction == CameraMovement::Left) { Position -= Right * velocity; }
    if (direction == CameraMovement::Right) { Position += Right * velocity; }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (Pitch > 89.0f) { Pitch = 89.0f; }
        if (Pitch < -89.0f) { Pitch = -89.0f; }
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) { Zoom = 1.0f; }
    if (Zoom > 45.0f) { Zoom = 45.0f; }
}

void Camera::updateCameraVectors()
{
    glm::vec3 front
    {
        cos(glm::radians(Yaw))* cos(glm::radians(Pitch)),
            sin(glm::radians(Pitch)),
            sin(glm::radians(Yaw))* cos(glm::radians(Pitch))
    };

    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}