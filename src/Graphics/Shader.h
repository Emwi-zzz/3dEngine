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
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    GLuint GetID() const { return m_RendererID; }

private:
    GLuint m_RendererID;
};
