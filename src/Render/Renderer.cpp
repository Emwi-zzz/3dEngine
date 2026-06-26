#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include "Core/Config.h"

Renderer::Renderer(unsigned int width, unsigned int height) {
    m_GBuffer = std::make_unique<GBuffer>(width, height);
    
    // We assume the caller or main loop will ensure these files exist
    m_GeometryShader = std::make_unique<Shader>("../assets/shaders/g_buffer.vert", "../assets/shaders/g_buffer.frag");
    m_LightingShader = std::make_unique<Shader>("../assets/shaders/deferred_light.vert", "../assets/shaders/deferred_light.frag");
    m_ParticleShader = std::make_unique<Shader>("../assets/shaders/particle.vert", "../assets/shaders/particle.frag");
    m_GodRaysShader = std::make_unique<Shader>("../assets/shaders/god_rays.vert", "../assets/shaders/god_rays.frag");
    m_ShadowShader = std::make_unique<Shader>("../assets/shaders/shadow.vert", "../assets/shaders/shadow.frag");

    m_LightingShader->Bind();
    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "gPosition"), 0);
    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "gNormal"), 1);
    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "gAlbedoSpec"), 2);
    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "shadowMap"), 3);
    m_LightingShader->Unbind();
    
    m_GodRaysShader->Bind();
    glUniform1i(glGetUniformLocation(m_GodRaysShader->GetID(), "screenTexture"), 0);
    m_GodRaysShader->Unbind();

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    setupQuad();
    setupScreenFBO(width, height);
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &screenFBO);
    glDeleteTextures(1, &screenColorBuffer);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
}

void Renderer::Resize(unsigned int width, unsigned int height) {
    m_GBuffer->Resize(width, height);
    glBindTexture(GL_TEXTURE_2D, screenColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
}

void Renderer::Draw(const Scene& scene, const Camera& camera, float width, float height, Config* config) {
    // Unbind any textures from previous frame to prevent feedback loops (Droste effect) on untextured meshes
    for (int i = 0; i < 4; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glm::vec3 lightDir(0.0f, -1.0f, 0.0f); // Default light dir

    // 0. Shadow Pass
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 75.0f;
    lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    // Center light view loosely around player or origin
    glm::vec3 lightTarget = camera.Position;
    lightView = glm::lookAt(lightTarget - lightDir * 20.0f, lightTarget, glm::vec3(0.0, 0.0, -1.0));
    lightSpaceMatrix = lightProjection * lightView;

    m_ShadowShader->Bind();
    m_ShadowShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // Render scene for depth
    scene.Draw(*m_ShadowShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 1. Geometry Pass: render scene's geometry/color data into gbuffer
    m_GBuffer->BindForWriting();
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), width / height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    
    m_GeometryShader->Bind();
    m_GeometryShader->SetMat4("projection", projection);
    m_GeometryShader->SetMat4("view", view);
    
    scene.Draw(*m_GeometryShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Lighting Pass (Render to Screen FBO)
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_LightingShader->Bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_GBuffer->gAlbedoSpec);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    
    // Set lighting uniforms
    m_LightingShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
    glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), "lightDir"), 1, &lightDir[0]);
    glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), "viewPos"), 1, &camera.Position[0]);
    
    // Configurable lighting
    glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), "ambientIntensity"), config->GetFloat("ambient_intensity", 0.1f));
    glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), "lightIntensity"), config->GetFloat("light_intensity", 1.0f));
    glm::vec3 lightCol = config->GetVec3("light_color", glm::vec3(1.0f));
    glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), "lightColor"), 1, &lightCol[0]);

    // Configurable fog
    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "fogMode"), (int)config->GetFloat("fog_mode", 2.0f));
    glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), "fogDensity"), config->GetFloat("fog_density", 0.05f));
    glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), "fogStart"), config->GetFloat("fog_start", 10.0f));
    glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), "fogEnd"), config->GetFloat("fog_end", 50.0f));
    glm::vec3 fogCol = config->GetVec3("fog_color", glm::vec3(0.5f, 0.6f, 0.7f));
    glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), "fogColor"), 1, &fogCol[0]);

    glUniform1i(glGetUniformLocation(m_LightingShader->GetID(), "numPointLights"), scene.GetPointLights().size());
    for (size_t i = 0; i < scene.GetPointLights().size() && i < 32; ++i) {
        const auto& pl = scene.GetPointLights()[i];
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), (prefix + "position").c_str()), 1, &pl->Position[0]);
        glUniform3fv(glGetUniformLocation(m_LightingShader->GetID(), (prefix + "color").c_str()), 1, &pl->Color[0]);
        glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), (prefix + "intensity").c_str()), pl->Intensity);
        glUniform1f(glGetUniformLocation(m_LightingShader->GetID(), (prefix + "radius").c_str()), pl->Radius);
    }

    renderQuad();

    // 3. Forward Pass for Particles
    m_GBuffer->BindForReading();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    m_ParticleShader->Bind();
    m_ParticleShader->SetMat4("projection", projection);
    m_ParticleShader->SetMat4("view", view);
    scene.DrawForward(*m_ParticleShader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // 4. God Rays Post-Processing (Render to Screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_GodRaysShader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenColorBuffer);

    glm::vec3 sunPosWorld = camera.Position + glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec4 sunPosClip = projection * view * glm::vec4(sunPosWorld, 1.0f);
    glm::vec2 sunPosScreen = glm::vec2(sunPosClip) / sunPosClip.w;
    sunPosScreen = sunPosScreen * 0.5f + 0.5f;

    glUniform2fv(glGetUniformLocation(m_GodRaysShader->GetID(), "sunPosition"), 1, &sunPosScreen[0]);

    renderQuad();
}

void Renderer::setupQuad() {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void Renderer::renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::setupScreenFBO(unsigned int width, unsigned int height) {
    glGenFramebuffers(1, &screenFBO);
    glGenTextures(1, &screenColorBuffer);
    
    glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
    glBindTexture(GL_TEXTURE_2D, screenColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorBuffer, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
