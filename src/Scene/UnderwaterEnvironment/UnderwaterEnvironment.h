#pragma once
#include "Scene/Entity.h"

class UnderwaterEnvironment : public Entity {
public:
    UnderwaterEnvironment(std::shared_ptr<Model> model);
    ~UnderwaterEnvironment() override = default;

    void ReloadConfig() override;
};
