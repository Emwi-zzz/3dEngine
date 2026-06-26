#include "Scene.h"

Scene::Scene() {}
Scene::~Scene() {}

void Scene::AddEntity(std::shared_ptr<Entity> entity)
{
    m_Entities.push_back(entity);
}

void Scene::Update(float dt)
{
    for (const auto& entity : m_Entities)
    {
        entity->Update(dt);
    }
}

void Scene::Draw(const Shader& shader) const
{
    for (const auto& entity : m_Entities)
    {
        entity->Draw(shader);
    }
}

void Scene::DrawForward(const Shader& shader) const
{
    for (const auto& entity : m_Entities)
    {
        entity->DrawForward(shader);
    }
}

void Scene::ReloadEntities()
{
    for (const auto& entity : m_Entities)
    {
        entity->ReloadConfig();
    }
}

bool Scene::CheckCollision(const AABB& aabb) const
{
    for (const auto& entity : m_Entities)
    {
        if (entity->HasCollision && CheckCollisionAABB(aabb, entity->GetGlobalAABB()))
        {
            return true;
        }
    }
    return false;
}

void Scene::CheckCollisionSphere(glm::vec3& sphereCenter, float radius) const
{
    for (const auto& entity : m_Entities)
    {
        entity->CheckCollision(sphereCenter, radius);
    }
}

void Scene::AddPointLight(std::shared_ptr<PointLight> light)
{
    m_PointLights.push_back(light);
}
