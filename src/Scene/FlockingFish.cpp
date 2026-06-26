#include "FlockingFish.h"

FlockingFish::FlockingFish(std::shared_ptr<Model> model, Scene* scene, std::vector<std::shared_ptr<FlockingFish>>* flock, glm::vec3 position)
    : Fish("LowPolyFish", model, scene, position), m_Flock(flock), m_Time(0.0f)
{
    BaseSpeed = 3.0f;
    MaxSpeed = 6.0f;
    AvoidanceRadius = 1.5f;
    Scale = glm::vec3(0.1f); // Increased scale
    RotationOffset = glm::vec3(90.0f, 0.0f, 0.0f);
    
    m_NeighborRadius = 5.0f;
    m_SeparationWeight = 1.5f;
    m_AlignmentWeight = 1.0f;
    m_CohesionWeight = 1.0f;
    
    // Random individuality
    m_IndividualityOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;
}

void FlockingFish::Update(float dt)
{
    m_Time += dt;
    
    glm::vec3 separation(0.0f);
    glm::vec3 alignment(0.0f);
    glm::vec3 cohesion(0.0f);
    
    int neighbors = 0;
    
    if (m_Flock) {
        for (const auto& other : *m_Flock) {
            if (other.get() == this || !other->IsActive) continue;
            
            float dist = glm::distance(Position, other->Position);
            if (dist < m_NeighborRadius && dist > 0.001f) {
                // Separation
                glm::vec3 diff = Position - other->Position;
                separation += glm::normalize(diff) / dist; 
                
                // Alignment
                alignment += other->Velocity;
                
                // Cohesion
                cohesion += other->Position;
                
                neighbors++;
            }
        }
    }
    
    if (neighbors > 0) {
        separation /= static_cast<float>(neighbors);
        
        alignment /= static_cast<float>(neighbors);
        if (glm::length(alignment) > 0.001f) {
            alignment = glm::normalize(alignment) * MaxSpeed - Velocity;
        }
        
        cohesion /= static_cast<float>(neighbors);
        cohesion = cohesion - Position;
        if (glm::length(cohesion) > 0.001f) {
            cohesion = glm::normalize(cohesion) * MaxSpeed - Velocity;
        }
    }
    
    // Add some individuality / randomness to look natural
    glm::vec3 individuality(
        glm::sin(m_Time * 2.0f + m_IndividualityOffset),
        glm::cos(m_Time * 1.5f + m_IndividualityOffset) * 0.5f,
        glm::sin(m_Time * 1.8f - m_IndividualityOffset)
    );
    
    Velocity += separation * m_SeparationWeight * dt * 5.0f;
    Velocity += alignment * m_AlignmentWeight * dt * 2.0f;
    Velocity += cohesion * m_CohesionWeight * dt * 2.0f;
    Velocity += individuality * dt * 1.0f;
    
    Fish::Update(dt);
}
