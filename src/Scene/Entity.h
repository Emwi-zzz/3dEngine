#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Graphics/Model.h"
#include "Core/Config.h"
#include "Core/Physics.h"

class Entity {
public:
    Entity(const std::string& name, std::shared_ptr<Model> model, glm::vec3 position = glm::vec3(0.0f));
    virtual ~Entity() = default;

    virtual void ReloadConfig() {}

    virtual void Update(float dt) {}
    virtual void Draw(const Shader& shader) const;
    virtual void DrawForward(const Shader& shader) const {}
    virtual AABB GetGlobalAABB() const;
    virtual void CheckCollision(glm::vec3& globalSphereCenter, float radius) const;

    std::string Name;
    bool HasCollision = true;
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;

    glm::mat4 GetModelMatrix() const;

private:
    std::shared_ptr<Model> m_Model;
};
