#pragma once
#include "Scene/Entity.h"
#include "Graphics/ParticleSystem.h"

#include "Scene/Scene.h"

class GoldCoin : public Entity {
public:
    GoldCoin(std::shared_ptr<Model> model, glm::vec3 position);

    void Update(float dt) override;

    std::shared_ptr<ParticleSystem> Bubbles;
    std::shared_ptr<PointLight> m_Light;

private:
    float m_Time;
    glm::vec3 m_BasePosition;
};
