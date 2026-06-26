#pragma once
#include <memory>
#include "GBuffer.h"
#include "Scene/Scene.h"
#include "Graphics/Shader.h"
#include "Render/Camera.h"

class Config;

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    ~Renderer();

    void Resize(unsigned int width, unsigned int height);
    void Draw(const Scene& scene, const Camera& camera, float width, float height, Config* config);

private:
    std::unique_ptr<GBuffer> m_GBuffer;
    std::unique_ptr<Shader> m_GeometryShader;
    std::unique_ptr<Shader> m_LightingShader;
    std::unique_ptr<Shader> m_ParticleShader;
    std::unique_ptr<Shader> m_GodRaysShader;
    std::unique_ptr<Shader> m_ShadowShader;

    unsigned int depthMapFBO;
    unsigned int depthMap;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    unsigned int quadVAO, quadVBO;
    unsigned int screenFBO, screenColorBuffer;

    void setupQuad();
    void renderQuad();
    void setupScreenFBO(unsigned int width, unsigned int height);
};
