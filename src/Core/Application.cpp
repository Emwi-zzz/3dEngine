#include "Core/Application.h"
#include "Render/Camera.h"
#include "Core/Config.h"
#include <iostream>

#ifdef ENABLE_UNDERWATER_ENVIRONMENT
#include "Scene/UnderwaterEnvironment/UnderwaterEnvironment.h"
#endif

#ifdef ENABLE_ENDLESS_FLOOR
#include "Scene/EndlessFloor/EndlessFloor.h"
#endif

#include "Graphics/ParticleSystem.h"

Application::Application(int width, int height, const std::string& title)
    : m_Width(width), m_Height(height), m_Title(title), m_Window(nullptr), m_IsRunning(false), 
      m_DeltaTime(0.0f), m_LastFrame(0.0f),
      m_LastX(width / 2.0f), m_LastY(height / 2.0f), m_FirstMouse(true)
{
}

Application::~Application()
{
    Shutdown();
}

void Application::Init()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(m_Window, MouseCallback);
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glViewport(0, 0, m_Width, m_Height);
    glEnable(GL_DEPTH_TEST);

    m_Config = std::make_unique<Config>("../config.txt");
    m_Camera = std::make_unique<Camera>(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f);
    m_Camera->MovementSpeed = m_Config->GetFloat("camera_speed", 2.5f);
    m_Renderer = std::make_unique<Renderer>(m_Width, m_Height);
    m_Scene = std::make_unique<Scene>();

    // Load user's model
#ifdef ENABLE_UNDERWATER_ENVIRONMENT
    auto envModel = std::make_shared<Model>("../models/underwater_environment/scene.gltf");
    
    auto envEntity1 = std::make_shared<UnderwaterEnvironment>(envModel);
    envEntity1->Position = glm::vec3(0.0f, -2.0f, -5.0f);
    envEntity1->Scale = glm::vec3(0.01f);
    envEntity1->ReloadConfig();
    m_Scene->AddEntity(envEntity1);

    auto envEntity2 = std::make_shared<UnderwaterEnvironment>(envModel);
    envEntity2->Position = glm::vec3(20.0f, -2.0f, 10.0f);
    envEntity2->Scale = glm::vec3(0.01f);
    envEntity2->Rotation = glm::vec3(0.0f, 180.0f, 0.0f);
    envEntity2->ReloadConfig();
    m_Scene->AddEntity(envEntity2);
#endif

#ifdef ENABLE_ENDLESS_FLOOR
    auto floor = std::make_shared<EndlessFloor>();
    m_Scene->AddEntity(floor);
#endif

    // Add Particle System
    auto bubbles = std::make_shared<ParticleSystem>(150, glm::vec3(0.0f, -1.0f, 0.0f), 20.0f);
    m_Scene->AddEntity(bubbles);
    
    // Add Point Lights
    PointLight pl1;
    pl1.Position = glm::vec3(0.0f, 1.0f, -5.0f);
    pl1.Color = glm::vec3(1.0f, 0.4f, 0.1f);
    pl1.Intensity = 3.0f;
    pl1.Radius = 15.0f;
    m_Scene->AddPointLight(pl1);

    PointLight pl2;
    pl2.Position = glm::vec3(10.0f, 0.0f, 5.0f);
    pl2.Color = glm::vec3(0.1f, 1.0f, 0.5f);
    pl2.Intensity = 5.0f;
    pl2.Radius = 10.0f;
    m_Scene->AddPointLight(pl2);
    
    m_IsRunning = true;
}

void Application::ProcessInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    glm::vec3 oldPos = m_Camera->Position;

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(0, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(1, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(2, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(3, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(4, m_DeltaTime);
    if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_Camera->ProcessKeyboard(5, m_DeltaTime);

    // Camera Collision check
    glm::vec3 targetPos = m_Camera->Position;
    m_Camera->Position = oldPos; // Revert to old

    float cameraRadius = 0.4f;

    m_Camera->Position.x = targetPos.x;
    m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);
    
    m_Camera->Position.y = targetPos.y;
    m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);
    
    m_Camera->Position.z = targetPos.z;
    m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);

    // Reload Config Hotkey
    if (glfwGetKey(m_Window, GLFW_KEY_R) == GLFW_PRESS)
    {
        m_Config = std::make_unique<Config>("../config.txt");
        m_Camera->MovementSpeed = m_Config->GetFloat("camera_speed", 2.5f);
        m_Scene->ReloadEntities();
    }

    float speedFast = m_Config->GetFloat("camera_speed_fast", 15.0f);
    float speedSlow = m_Config->GetFloat("camera_speed_slow", 0.5f);

    if (glfwGetKey(m_Window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(m_Window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    {
        m_Camera->MovementSpeed += 15.0f * m_DeltaTime;
        if (m_Camera->MovementSpeed > speedFast) m_Camera->MovementSpeed = speedFast;
    }
    else if (glfwGetKey(m_Window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(m_Window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    {
        m_Camera->MovementSpeed -= 15.0f * m_DeltaTime;
        if (m_Camera->MovementSpeed < speedSlow) m_Camera->MovementSpeed = speedSlow;
    }
}

void Application::Update()
{
    float currentFrame = glfwGetTime();
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;

    if (glfwWindowShouldClose(m_Window)) {
        m_IsRunning = false;
    }
    
    if (m_Scene) {
        m_Scene->Update(m_DeltaTime);
    }
}

void Application::Render()
{
    m_Renderer->Draw(*m_Scene, *m_Camera, m_Width, m_Height, m_Config.get());
    glfwSwapBuffers(m_Window);
}

void Application::Shutdown()
{
    m_Scene.reset();
    m_Renderer.reset();
    m_Camera.reset();

    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

void Application::Run()
{
    Init();

    while (m_IsRunning) {
        glfwPollEvents();
        Update();
        ProcessInput();
        Render();
    }
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->m_Width = width;
    app->m_Height = height;
    
    // We update viewport in the geometry pass in Renderer::Draw, but we can do it here too just in case.
    if (app->m_Renderer) {
        app->m_Renderer->Resize(width, height);
    }
}

void Application::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (app->m_FirstMouse)
    {
        app->m_LastX = xpos;
        app->m_LastY = ypos;
        app->m_FirstMouse = false;
    }

    float xoffset = xpos - app->m_LastX;
    float yoffset = app->m_LastY - ypos; 

    app->m_LastX = xpos;
    app->m_LastY = ypos;

    if (app->m_Camera)
        app->m_Camera->ProcessMouseMovement(xoffset, yoffset);
}
