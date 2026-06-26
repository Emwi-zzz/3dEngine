#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include "Scene/Scene.h"
#include "Render/Renderer.h"
#include "Render/Camera.h"
#include "Core/Config.h"

class Application {
public:
    Application(int width, int height, const std::string& title);
    ~Application();

    void Run();

private:
    void Init();
    void ProcessInput();
    void Update();
    void Render();
    void Shutdown();

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);

    int m_Width;
    int m_Height;
    std::string m_Title;
    GLFWwindow* m_Window;
    bool m_IsRunning;

    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Renderer> m_Renderer;
    std::unique_ptr<Camera> m_Camera;
    std::unique_ptr<Config> m_Config;

    float m_DeltaTime;
    float m_LastFrame;

    float m_LastX;
    float m_LastY;
    bool m_FirstMouse;

    int m_CoinsCollected = 0;
    bool m_PlayerCollisionEnabled = true;
    bool m_CKeyPressed = false;
    bool m_HintEnabled = false;
    bool m_HKeyPressed = false;
    std::vector<std::shared_ptr<class GoldCoin>> m_GoldCoins;
    std::vector<std::shared_ptr<class FlockingFish>> m_Flock;
};
