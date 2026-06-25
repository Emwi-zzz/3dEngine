#include "VertexArray.h"

VertexArray::VertexArray() { glGenVertexArrays(1, &m_RendererID); }
VertexArray::~VertexArray() { glDeleteVertexArrays(1, &m_RendererID); }
void VertexArray::Bind() const { glBindVertexArray(m_RendererID); }
void VertexArray::Unbind() const { glBindVertexArray(0); }

void VertexArray::AddBuffer(const VertexBuffer& vb, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    Bind();
    vb.Bind();
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
    glEnableVertexAttribArray(index);
}
