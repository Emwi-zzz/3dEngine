#pragma once
#include <glad/gl.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    GLuint GetID() const { return m_RendererID; }

private:
    GLuint m_RendererID;
};
