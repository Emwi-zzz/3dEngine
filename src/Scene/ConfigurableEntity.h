#pragma once
#include "Scene/Entity.h"
#include "Core/Config.h"

class ConfigurableEntity : public Entity {
public:
    ConfigurableEntity(const std::string& name, std::shared_ptr<Model> model, const std::string& configPath);
    ~ConfigurableEntity() override = default;

    void ReloadConfig() override;

private:
    std::string m_ConfigPath;
};
