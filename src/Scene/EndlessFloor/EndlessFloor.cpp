#include "EndlessFloor.h"
#include <glad/gl.h>

EndlessFloor::EndlessFloor()
    : Entity("EndlessFloor", nullptr)
{
    HasCollision = true;
    m_GridShader = std::make_unique<Shader>("../assets/shaders/grid.vert", "../assets/shaders/grid.frag");

    float vertices[] = {
        -10000.0f, 0.0f, -10000.0f,
         10000.0f, 0.0f, -10000.0f,
        -10000.0f, 0.0f,  10000.0f,
         10000.0f, 0.0f,  10000.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

EndlessFloor::~EndlessFloor()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void EndlessFloor::Draw(const Shader& shader) const
{
    glm::mat4 proj, view;
    glGetUniformfv(shader.GetID(), glGetUniformLocation(shader.GetID(), "projection"), &proj[0][0]);
    glGetUniformfv(shader.GetID(), glGetUniformLocation(shader.GetID(), "view"), &view[0][0]);

    m_GridShader->Bind();
    m_GridShader->SetMat4("projection", proj);
    m_GridShader->SetMat4("view", view);
    m_GridShader->SetMat4("model", GetModelMatrix());

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    shader.Bind();
}

AABB EndlessFloor::GetGlobalAABB() const
{
    return {glm::vec3(-10000.0f, -10.0f, -10000.0f), glm::vec3(10000.0f, 0.0f, 10000.0f)};
}

void EndlessFloor::CheckCollision(glm::vec3& globalSphereCenter, float radius) const
{
    if (globalSphereCenter.y - radius < 0.0f) {
        if (globalSphereCenter.x >= -10000.0f && globalSphereCenter.x <= 10000.0f &&
            globalSphereCenter.z >= -10000.0f && globalSphereCenter.z <= 10000.0f) {
            
            globalSphereCenter.y = radius;
        }
    }
}
