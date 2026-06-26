#pragma once
#include "Fish.h"
#include <vector>

class FlockingFish : public Fish {
public:
    FlockingFish(std::shared_ptr<Model> model, Scene* scene, std::vector<std::shared_ptr<FlockingFish>>* flock, glm::vec3 position = glm::vec3(0.0f));
    virtual ~FlockingFish() = default;

    virtual void Update(float dt) override;

private:
    std::vector<std::shared_ptr<FlockingFish>>* m_Flock;
    float m_Time;
    
    // Flocking parameters
    float m_NeighborRadius;
    float m_SeparationWeight;
    float m_AlignmentWeight;
    float m_CohesionWeight;
    
    // Slight individuality
    float m_IndividualityOffset;
};
