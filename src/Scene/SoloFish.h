#pragma once
#include "Fish.h"

class SeaTurtle : public Fish {
public:
    SeaTurtle(std::shared_ptr<Model> model, Scene* scene, glm::vec3 position = glm::vec3(0.0f));
    virtual ~SeaTurtle() = default;

    virtual void Update(float dt) override;
private:
    float m_Time;
};

class DiscusFish : public Fish {
public:
    DiscusFish(std::shared_ptr<Model> model, Scene* scene, glm::vec3 position = glm::vec3(0.0f));
    virtual ~DiscusFish() = default;

    virtual void Update(float dt) override;

    std::shared_ptr<PointLight> m_Light;
private:
    float m_Time;
};
