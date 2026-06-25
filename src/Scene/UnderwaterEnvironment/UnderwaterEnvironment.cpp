#include "UnderwaterEnvironment.h"

UnderwaterEnvironment::UnderwaterEnvironment(std::shared_ptr<Model> model)
    : Entity("UnderwaterEnvironment", model)
{
}

void UnderwaterEnvironment::ReloadConfig()
{
    Config config("../configs/underwater_environment.txt");
    Position = config.GetTransformVec3("pos", Position);
    Rotation = config.GetTransformVec3("rot", Rotation);
    Scale = config.GetTransformVec3("scale", Scale);
}
