#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
{
    Position = position;
    WorldUp = up;
    
    glm::vec3 eulerAngles(glm::radians(pitch), glm::radians(yaw), 0.0f);
    Orientation = glm::quat(eulerAngles);
    
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(int direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == 0) // FORWARD
        Position += Front * velocity;
    if (direction == 1) // BACKWARD
        Position -= Front * velocity;
    if (direction == 2) // LEFT
        Position -= Right * velocity;
    if (direction == 3) // RIGHT
        Position += Right * velocity;
    if (direction == 4) // UP
        Position += WorldUp * velocity;
    if (direction == 5) // DOWN
        Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    glm::quat pitchQuat = glm::angleAxis(glm::radians(yoffset), Right);
    glm::quat yawQuat   = glm::angleAxis(glm::radians(-xoffset), WorldUp);

    Orientation = yawQuat * pitchQuat * Orientation;
    Orientation = glm::normalize(Orientation);

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    Front = glm::normalize(Orientation * glm::vec3(0.0f, 0.0f, -1.0f));
    Up    = glm::normalize(Orientation * glm::vec3(0.0f, 1.0f, 0.0f));
    Right = glm::normalize(Orientation * glm::vec3(1.0f, 0.0f, 0.0f));
}
