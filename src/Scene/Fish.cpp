#include "Fish.h"
#include <glm/gtc/quaternion.hpp>
#include <iostream>

Fish::Fish(const std::string& name, std::shared_ptr<Model> model, Scene* scene, glm::vec3 position)
    : Entity(name, model, position), m_Scene(scene), Velocity(1.0f, 0.0f, 0.0f), 
      RotationOffset(0.0f), BaseSpeed(2.0f), MaxSpeed(4.0f), AvoidanceRadius(2.0f)
{
    HasCollision = false; // Fish themselves won't act as obstacles for other things to bounce off statically
}

void Fish::Update(float dt)
{
    if (!IsActive) return;

    // Default fish movement: just go forward with velocity
    // Avoid obstacles
    glm::vec3 avoidance = CalculateObstacleAvoidance();
    Velocity += avoidance * dt * 10.0f;

    if (m_Scene) {
        float distToPlayer = glm::distance(Position, m_Scene->PlayerPosition);
        if (distToPlayer > 10.0f) {
            glm::vec3 dirToPlayer = glm::normalize(m_Scene->PlayerPosition - Position);
            Velocity += dirToPlayer * dt * 2.0f; // Attract to player
        }
    }

    // Clamp speed
    float speed = glm::length(Velocity);
    if (speed > 0.0f) {
        if (speed > MaxSpeed) speed = MaxSpeed;
        else if (speed < BaseSpeed) speed = BaseSpeed;
        Velocity = (Velocity / glm::length(Velocity)) * speed;
    }

    Position += Velocity * dt;
    UpdateRotation();
}

void Fish::UpdateRotation()
{
    if (glm::length(Velocity) > 0.01f) {
        glm::vec3 direction = glm::normalize(Velocity);
        float yaw = glm::degrees(atan2(direction.x, direction.z));
        float pitch = glm::degrees(asin(-direction.y));
        Rotation.y = yaw + RotationOffset.y;
        Rotation.x = pitch + RotationOffset.x;
        Rotation.z = RotationOffset.z;
    }
}

glm::vec3 Fish::CalculateObstacleAvoidance()
{
    if (!m_Scene) return glm::vec3(0.0f);

    glm::vec3 dir = glm::normalize(Velocity);
    if (glm::length(dir) < 0.01f) dir = glm::vec3(1.0f, 0.0f, 0.0f);

    // Cast a sphere forward to check for collision
    glm::vec3 probeCenter = Position + dir * AvoidanceRadius;
    glm::vec3 originalCenter = probeCenter;

    m_Scene->CheckCollisionSphere(probeCenter, AvoidanceRadius * 0.5f);

    glm::vec3 avoidanceForce(0.0f);
    if (probeCenter != originalCenter) {
        // We hit something, probeCenter has been pushed away
        glm::vec3 pushOutDir = probeCenter - originalCenter;
        
        // Add a strong force in the push-out direction
        avoidanceForce = glm::normalize(pushOutDir) * BaseSpeed * 5.0f;
        
        // Also steer slightly up to avoid getting stuck on floors
        avoidanceForce.y += BaseSpeed; 
    }

    // Ensure fish stay somewhat within bounds (e.g. y > 0)
    if (Position.y < 0.5f) {
        avoidanceForce.y += (0.5f - Position.y) * 5.0f;
    }

    return avoidanceForce;
}
