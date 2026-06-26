#pragma once
#include "Entity.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Fish : public Entity {
public:
    Fish(const std::string& name, std::shared_ptr<Model> model, Scene* scene, glm::vec3 position = glm::vec3(0.0f));
    virtual ~Fish() = default;

    virtual void Update(float dt) override;
    
    glm::vec3 Velocity;
    glm::vec3 RotationOffset;
    float BaseSpeed;
    float MaxSpeed;
    float AvoidanceRadius;

protected:
    Scene* m_Scene;

    // Helper for rotating towards velocity
    void UpdateRotation();
    
    // Calculates a vector that steers away from obstacles
    glm::vec3 CalculateObstacleAvoidance();
};
