#include "ParticleSystem.h"
#include <glad/gl.h>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>

ParticleSystem::ParticleSystem(int count, glm::vec3 origin, float spread)
    : Entity("ParticleSystem", nullptr, origin), m_Origin(origin), m_Spread(spread)
{
    Name = "ParticleSystem";
    HasCollision = false;

    for (int i = 0; i < count; ++i) {
        Particle p;
        RespawnParticle(p);
        p.Life = static_cast<float>(rand()) / RAND_MAX * p.MaxLife;
        m_Particles.push_back(p);
    }

    float quadVertices[] = {
        // pos        
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, m_Particles.size() * (sizeof(glm::mat4) + sizeof(glm::vec4)), nullptr, GL_DYNAMIC_DRAW);

    std::size_t vec4Size = sizeof(glm::vec4);
    std::size_t stride = sizeof(glm::mat4) + sizeof(glm::vec4);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(vec4Size));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)(2 * vec4Size));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * vec4Size));
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * vec4Size));
    glVertexAttribDivisor(5, 1);

    glBindVertexArray(0);
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
}

void ParticleSystem::RespawnParticle(Particle& p) {
    float rx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * m_Spread;
    float rz = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * m_Spread;
    float ry = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * (m_Spread * 0.2f);
    
    p.Position = m_Origin + glm::vec3(rx, ry, rz);
    
    float vx = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.2f;
    float vy = (static_cast<float>(rand()) / RAND_MAX) * 0.5f + 0.2f;
    float vz = ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.2f;
    
    p.Velocity = glm::vec3(vx, vy, vz);
    p.Color = glm::vec4(0.8f, 0.9f, 1.0f, 0.6f); 
    p.Life = 0.0f;
    p.MaxLife = (static_cast<float>(rand()) / RAND_MAX) * 5.0f + 3.0f;
    p.Size = (static_cast<float>(rand()) / RAND_MAX) * 0.05f + 0.02f;
}

void ParticleSystem::Update(float dt) {
    for (auto& p : m_Particles) {
        p.Life += dt;
        if (p.Life >= p.MaxLife) {
            RespawnParticle(p);
        }
        p.Position += p.Velocity * dt;
        p.Position.x += sin(p.Life * 5.0f) * 0.005f;
    }
}

void ParticleSystem::DrawForward(const Shader& shader) const {
    std::vector<float> instanceData;
    instanceData.reserve(m_Particles.size() * 20);

    for (const auto& p : m_Particles) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.Position);
        model = glm::scale(model, glm::vec3(p.Size));
        
        const float* m = (const float*)glm::value_ptr(model);
        instanceData.insert(instanceData.end(), m, m + 16);
        instanceData.push_back(p.Color.r);
        instanceData.push_back(p.Color.g);
        instanceData.push_back(p.Color.b);
        float alpha = p.Color.a * (1.0f - (p.Life / p.MaxLife));
        instanceData.push_back(alpha);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceData.size() * sizeof(float), instanceData.data());

    shader.Bind();
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_Particles.size());
    glBindVertexArray(0);
}
