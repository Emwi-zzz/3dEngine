#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Config {
public:
    Config(const std::string& filepath);

    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    glm::vec3 GetVec3(const std::string& keyPrefix, glm::vec3 defaultValue = glm::vec3(0.0f)) const;
    glm::vec3 GetTransformVec3(const std::string& keyPrefix, glm::vec3 defaultValue = glm::vec3(0.0f)) const;

private:
    std::unordered_map<std::string, float> m_Values;
};
