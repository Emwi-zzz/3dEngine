#include "GoldCoin.h"

GoldCoin::GoldCoin(std::shared_ptr<Model> model, glm::vec3 position)
    : Entity("GoldCoin", model, position), m_Time(0.0f), m_BasePosition(position)
{
    Scale = glm::vec3(0.05f); // Adjust scale to fit the world, may need tuning
    HasCollision = false; // We will use distance based collection
}

void GoldCoin::Update(float dt)
{
    if (!IsActive) {
        if (m_Light) m_Light->Intensity = 0.0f;
        if (Bubbles) Bubbles->IsActive = false;
        return;
    }
    m_Time += dt;

    // Rotate 90 degrees per second
    Rotation.y += dt * 90.0f;

    // Bob up and down (sinus)
    Position.y = m_BasePosition.y + glm::sin(m_Time * 2.0f) * 0.25f;

    // Update bubbles position to follow coin
    if (Bubbles) {
        Bubbles->Position = Position - glm::vec3(0.0f, 0.5f, 0.0f);
    }

    if (m_Light) {
        m_Light->Position = Position;
    }
}
