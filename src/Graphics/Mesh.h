#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Core/Physics.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>> textures;
    AABB localAABB;
    std::vector<BVHNode> bvhNodes;
    std::vector<int> bvhTriIndices;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures);
    void Draw(const Shader& shader) const;
    void CheckCollision(glm::vec3& localSphereCenter, float localRadius) const;

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
    void buildBVH();
    void updateNodeBounds(int nodeIdx);
    void subdivide(int nodeIdx);
    void checkCollisionBVH(int nodeIdx, glm::vec3& localSphereCenter, float localRadius) const;
};
