#pragma once
#include <glad/gl.h>

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

private:
    GLuint m_RendererID;
};

class IndexBuffer {
public:
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;
    unsigned int GetCount() const { return m_Count; }

private:
    GLuint m_RendererID;
    unsigned int m_Count;
};
