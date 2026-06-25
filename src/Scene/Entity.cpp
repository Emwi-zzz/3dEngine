#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>

Entity::Entity(const std::string& name, std::shared_ptr<Model> model, glm::vec3 position)
    : Name(name), m_Model(model), Position(position), Rotation(0.0f), Scale(1.0f)
{
}

void Entity::Draw(const Shader& shader) const
{
    if (m_Model)
    {
        shader.SetMat4("model", GetModelMatrix());
        m_Model->Draw(shader);
    }
}

glm::mat4 Entity::GetModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, Position);
    model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, Scale);
    return model;
}

AABB Entity::GetGlobalAABB() const
{
    if (!m_Model) return {glm::vec3(0.0f), glm::vec3(0.0f)};
    
    AABB local = m_Model->localAABB;
    glm::mat4 modelMat = GetModelMatrix();
    
    glm::vec3 corners[8] = {
        glm::vec3(local.Min.x, local.Min.y, local.Min.z),
        glm::vec3(local.Max.x, local.Min.y, local.Min.z),
        glm::vec3(local.Min.x, local.Max.y, local.Min.z),
        glm::vec3(local.Max.x, local.Max.y, local.Min.z),
        glm::vec3(local.Min.x, local.Min.y, local.Max.z),
        glm::vec3(local.Max.x, local.Min.y, local.Max.z),
        glm::vec3(local.Min.x, local.Max.y, local.Max.z),
        glm::vec3(local.Max.x, local.Max.y, local.Max.z)
    };
    
    glm::vec3 globalMin(std::numeric_limits<float>::max());
    glm::vec3 globalMax(std::numeric_limits<float>::lowest());
    
    for (int i = 0; i < 8; i++) {
        glm::vec4 transformed = modelMat * glm::vec4(corners[i], 1.0f);
        globalMin = glm::min(globalMin, glm::vec3(transformed));
        globalMax = glm::max(globalMax, glm::vec3(transformed));
    }
    
    return {globalMin, globalMax};
}

void Entity::CheckCollision(glm::vec3& globalSphereCenter, float radius) const
{
    if (!HasCollision || !m_Model) return;

    AABB globalAABB = GetGlobalAABB();
    if (!CheckCollisionAABBSphere(globalAABB, globalSphereCenter, radius)) return;

    glm::mat4 modelMat = GetModelMatrix();
    glm::mat4 invModelMat = glm::inverse(modelMat);
    
    glm::vec3 localCenter = glm::vec3(invModelMat * glm::vec4(globalSphereCenter, 1.0f));
    
    float maxScale = glm::max(Scale.x, glm::max(Scale.y, Scale.z));
    if (maxScale == 0.0f) return;
    float localRadius = radius / maxScale; 

    glm::vec3 origLocalCenter = localCenter;
    
    m_Model->CheckCollision(localCenter, localRadius);
    
    if (localCenter != origLocalCenter) {
        glm::vec3 localDelta = localCenter - origLocalCenter;
        glm::mat3 normalMat = glm::mat3(modelMat); 
        glm::vec3 globalDelta = normalMat * localDelta;
        globalSphereCenter += globalDelta;
    }
}
