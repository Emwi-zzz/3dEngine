#pragma once
#include <glad/gl.h>

class GBuffer {
public:
    GBuffer(unsigned int width, unsigned int height);
    ~GBuffer();

    void BindForWriting();
    void BindForReading();
    
    void Resize(unsigned int width, unsigned int height);

    GLuint gPosition, gNormal, gAlbedoSpec;

private:
    GLuint m_FBO;
    GLuint m_DepthRenderBuffer;
    void setup(unsigned int width, unsigned int height);
    void cleanup();
};
