#pragma once
#include <glad/gl.h>
#include "Buffer.h"

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddBuffer(const VertexBuffer& vb, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);

private:
    GLuint m_RendererID;
};
