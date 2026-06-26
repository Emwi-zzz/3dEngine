#include "SoloFish.h"
#include <glm/gtc/color_space.hpp>

// --- SeaTurtle ---
SeaTurtle::SeaTurtle(std::shared_ptr<Model> model, Scene* scene, glm::vec3 position)
    : Fish("SeaTurtle", model, scene, position), m_Time(0.0f)
{
    BaseSpeed = 1.0f;
    MaxSpeed = 2.0f;
    AvoidanceRadius = 3.0f;
    Scale = glm::vec3(2.0f); // Increased scale
    RotationOffset = glm::vec3(0.0f, 180.0f, 0.0f);
}

void SeaTurtle::Update(float dt)
{
    m_Time += dt;
    
    // Add some slow wandering behavior
    Velocity.x += glm::sin(m_Time * 0.5f) * dt * 0.5f;
    Velocity.z += glm::cos(m_Time * 0.33f) * dt * 0.5f;
    Velocity.y += glm::sin(m_Time * 0.2f) * dt * 0.2f;

    Fish::Update(dt);
}

// --- DiscusFish ---
DiscusFish::DiscusFish(std::shared_ptr<Model> model, Scene* scene, glm::vec3 position)
    : Fish("DiscusFish", model, scene, position), m_Time(0.0f)
{
    BaseSpeed = 1.5f;
    MaxSpeed = 3.0f;
    AvoidanceRadius = 1.5f;
    Scale = glm::vec3(2.5f); // Increased scale
}

void DiscusFish::Update(float dt)
{
    m_Time += dt;

    // Wandering behavior
    Velocity.x += glm::sin(m_Time * 1.5f) * dt;
    Velocity.z += glm::cos(m_Time * 1.1f) * dt;
    Velocity.y += glm::sin(m_Time * 0.8f) * dt * 0.5f;

    Fish::Update(dt);

    if (m_Light) {
        m_Light->Position = Position;
        
        // Pulsing glow and color switch
        // HSV to RGB for smooth color transition
        float hue = fmod(m_Time * 30.0f, 360.0f); // Change color over time
        glm::vec3 hsv(hue, 1.0f, 1.0f);
        
        // Manual HSV to RGB conversion
        float s = hsv.y;
        float v = hsv.z;
        float c = v * s;
        float x = c * (1.0f - std::abs(fmod(hue / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        glm::vec3 rgb;
        
        if (hue >= 0 && hue < 60) rgb = glm::vec3(c, x, 0);
        else if (hue >= 60 && hue < 120) rgb = glm::vec3(x, c, 0);
        else if (hue >= 120 && hue < 180) rgb = glm::vec3(0, c, x);
        else if (hue >= 180 && hue < 240) rgb = glm::vec3(0, x, c);
        else if (hue >= 240 && hue < 300) rgb = glm::vec3(x, 0, c);
        else rgb = glm::vec3(c, 0, x);
        
        rgb += glm::vec3(m);

        m_Light->Color = rgb;
        m_Light->Intensity = 2.0f + glm::sin(m_Time * 5.0f) * 1.5f; // Reduced intensity
        m_Light->Radius = 15.0f; // Larger radius so we can see it
    }
}
