#pragma once
#include <vector>
#include <memory>
#include "Entity.h"
#include "Core/Config.h"
#include "Core/Physics.h"

struct PointLight {
    glm::vec3 Position;
    glm::vec3 Color;
    float Intensity;
    float Radius;
};

class Scene {
public:
    Scene();
    ~Scene();

    glm::vec3 PlayerPosition = glm::vec3(0.0f);

    void AddEntity(std::shared_ptr<Entity> entity);
    void Update(float dt);
    void Draw(const Shader& shader) const;
    void DrawForward(const Shader& shader) const;
    void ReloadEntities();
    bool CheckCollision(const AABB& aabb) const;
    void CheckCollisionSphere(glm::vec3& sphereCenter, float radius) const;

    void AddPointLight(std::shared_ptr<PointLight> light);
    const std::vector<std::shared_ptr<PointLight>>& GetPointLights() const { return m_PointLights; }

private:
    std::vector<std::shared_ptr<Entity>> m_Entities;
    std::vector<std::shared_ptr<PointLight>> m_PointLights;
};
