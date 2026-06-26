#include "ConfigurableEntity.h"

ConfigurableEntity::ConfigurableEntity(const std::string& name, std::shared_ptr<Model> model, const std::string& configPath)
    : Entity(name, model), m_ConfigPath(configPath)
{
    HasCollision = false;
}

void ConfigurableEntity::ReloadConfig()
{
    Config config(m_ConfigPath);
    Position = config.GetTransformVec3("pos", Position);
    Rotation = config.GetTransformVec3("rot", Rotation);
    Scale = config.GetTransformVec3("scale", Scale);
    HasCollision = config.GetFloat("has_collision", HasCollision ? 1.0f : 0.0f) > 0.5f;
}
