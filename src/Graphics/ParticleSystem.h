#pragma once
#include "Scene/Entity.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;
    float Life;
    float MaxLife;
    float Size;
};

class ParticleSystem : public Entity {
public:
    ParticleSystem(int count, glm::vec3 origin, float spread);
    ~ParticleSystem() override;

    void Update(float dt) override;
    void DrawForward(const Shader& shader) const override;
    AABB GetGlobalAABB() const override { return AABB(); }

private:
    std::vector<Particle> m_Particles;
    unsigned int VAO, VBO, instanceVBO;
    glm::vec3 m_Origin;
    float m_Spread;

    void RespawnParticle(Particle& particle);
};
