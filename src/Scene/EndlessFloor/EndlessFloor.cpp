#include "EndlessFloor.h"
#include <glad/gl.h>
#include <glm/gtc/noise.hpp>
#include <vector>

float EndlessFloor::GetHeightAt(float x, float z)
{
    float height = 0.0f;
    // Sinus waves for dunes
    height += glm::sin(x * 0.1f) * glm::cos(z * 0.1f) * 2.0f;
    height += glm::sin(x * 0.5f + z * 0.2f) * 0.5f;
    
    // Noise for fine sand detail and randomness
    height += glm::simplex(glm::vec2(x * 0.05f, z * 0.05f)) * 3.0f;
    height += glm::simplex(glm::vec2(x * 0.2f, z * 0.2f)) * 0.5f;
    
    return height - 2.0f; // slightly lower than 0 to fit with other models
}

EndlessFloor::EndlessFloor()
    : Entity("EndlessFloor", nullptr)
{
    HasCollision = true;
    m_SandShader = std::make_unique<Shader>("../assets/shaders/sand.vert", "../assets/shaders/sand.frag");

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    int gridSize = 1000; // 1000x1000 grid
    float step = 1.0f;
    float startX = - (gridSize * step) / 2.0f;
    float startZ = - (gridSize * step) / 2.0f;

    // Generate vertices (position and normal)
    for (int z = 0; z <= gridSize; ++z) {
        for (int x = 0; x <= gridSize; ++x) {
            float worldX = startX + x * step;
            float worldZ = startZ + z * step;
            float worldY = GetHeightAt(worldX, worldZ);
            
            // Calculate normal via finite difference
            float delta = 0.01f;
            float hL = GetHeightAt(worldX - delta, worldZ);
            float hR = GetHeightAt(worldX + delta, worldZ);
            float hD = GetHeightAt(worldX, worldZ - delta);
            float hU = GetHeightAt(worldX, worldZ + delta);
            
            glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f * delta, hD - hU));

            vertices.push_back(worldX);
            vertices.push_back(worldY);
            vertices.push_back(worldZ);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // Generate indices
    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            unsigned int topLeft = z * (gridSize + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * (gridSize + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    m_IndexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

EndlessFloor::~EndlessFloor()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void EndlessFloor::Draw(const Shader& shader) const
{
    // If the shader being used is the shadow map shader (which has lightSpaceMatrix), use it!
    if (glGetUniformLocation(shader.GetID(), "lightSpaceMatrix") != -1) {
        shader.SetMat4("model", GetModelMatrix());
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        return;
    }

    glm::mat4 proj, view;
    glGetUniformfv(shader.GetID(), glGetUniformLocation(shader.GetID(), "projection"), &proj[0][0]);
    glGetUniformfv(shader.GetID(), glGetUniformLocation(shader.GetID(), "view"), &view[0][0]);

    m_SandShader->Bind();
    m_SandShader->SetMat4("projection", proj);
    m_SandShader->SetMat4("view", view);
    m_SandShader->SetMat4("model", GetModelMatrix());

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    shader.Bind();
}

AABB EndlessFloor::GetGlobalAABB() const
{
    return {glm::vec3(-10000.0f, -50.0f, -10000.0f), glm::vec3(10000.0f, 50.0f, 10000.0f)};
}

void EndlessFloor::CheckCollision(glm::vec3& globalSphereCenter, float radius) const
{
    float groundHeight = GetHeightAt(globalSphereCenter.x, globalSphereCenter.z);
    
    if (globalSphereCenter.y - radius < groundHeight) {
        globalSphereCenter.y = groundHeight + radius;
    }
}
