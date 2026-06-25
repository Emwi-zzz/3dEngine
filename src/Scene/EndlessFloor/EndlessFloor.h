#pragma once
#include "Scene/Entity.h"
#include "Graphics/Shader.h"
#include <memory>

class EndlessFloor : public Entity {
public:
    EndlessFloor();
    ~EndlessFloor() override;

    void Draw(const Shader& shader) const override;
    AABB GetGlobalAABB() const override;
    void CheckCollision(glm::vec3& globalSphereCenter, float radius) const override;

private:
    unsigned int VAO, VBO;
    std::unique_ptr<Shader> m_GridShader;
};
