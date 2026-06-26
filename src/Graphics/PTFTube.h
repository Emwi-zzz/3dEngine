#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include "Graphics/Shader.h"
#include "Scene/Entity.h"

class PTFTube : public Entity {
public:
    PTFTube(const std::vector<glm::vec3>& controlPoints, float radius, int segments, int sides);
    ~PTFTube();

    void Draw(const Shader& shader) const override;

private:
    struct PTFVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    void setupMesh(const std::vector<PTFVertex>& vertices, const std::vector<unsigned int>& indices);
    glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);
};
