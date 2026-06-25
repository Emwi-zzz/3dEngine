#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config::Config(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file " << filepath << " - using defaults." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string valueStr = line.substr(delimiterPos + 1);
            try {
                m_Values[key] = std::stof(valueStr);
            } catch (...) {
                std::cerr << "Error parsing config value for key: " << key << std::endl;
            }
        }
    }
}

float Config::GetFloat(const std::string& key, float defaultValue) const
{
    auto it = m_Values.find(key);
    if (it != m_Values.end())
        return it->second;
    return defaultValue;
}

glm::vec3 Config::GetVec3(const std::string& keyPrefix, glm::vec3 defaultValue) const
{
    glm::vec3 result = defaultValue;
    if (m_Values.find(keyPrefix + "_r") != m_Values.end()) result.x = m_Values.at(keyPrefix + "_r");
    if (m_Values.find(keyPrefix + "_g") != m_Values.end()) result.y = m_Values.at(keyPrefix + "_g");
    if (m_Values.find(keyPrefix + "_b") != m_Values.end()) result.z = m_Values.at(keyPrefix + "_b");
    return result;
}

glm::vec3 Config::GetTransformVec3(const std::string& keyPrefix, glm::vec3 defaultValue) const
{
    glm::vec3 result = defaultValue;
    if (m_Values.find(keyPrefix + "_x") != m_Values.end()) result.x = m_Values.at(keyPrefix + "_x");
    if (m_Values.find(keyPrefix + "_y") != m_Values.end()) result.y = m_Values.at(keyPrefix + "_y");
    if (m_Values.find(keyPrefix + "_z") != m_Values.end()) result.z = m_Values.at(keyPrefix + "_z");
    return result;
}
