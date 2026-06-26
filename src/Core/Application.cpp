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
#include "Scene/GoldCoin.h"
#include "Scene/SoloFish.h"
#include "Scene/FlockingFish.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
    auto pl1 = std::make_shared<PointLight>();
    pl1->Position = glm::vec3(0.0f, 1.0f, -5.0f);
    pl1->Color = glm::vec3(1.0f, 0.4f, 0.1f);
    pl1->Intensity = 3.0f;
    pl1->Radius = 15.0f;
    m_Scene->AddPointLight(pl1);

    auto pl2 = std::make_shared<PointLight>();
    pl2->Position = glm::vec3(10.0f, 0.0f, 5.0f);
    pl2->Color = glm::vec3(0.1f, 1.0f, 0.5f);
    pl2->Intensity = 5.0f;
    pl2->Radius = 10.0f;
    m_Scene->AddPointLight(pl2);
    
    // Add Gold Coins
    std::vector<glm::vec3> coinPositions = {
        glm::vec3(2.0f, 0.5f, -2.0f),
        glm::vec3(5.0f, 0.5f, -3.0f),
        glm::vec3(-3.0f, 0.5f, 4.0f),
        glm::vec3(-6.0f, 0.5f, -1.0f),
        // 20 more positions within a 50x50 area
        glm::vec3(12.5f, 0.5f, -18.2f),
        glm::vec3(-22.1f, 0.5f, 8.4f),
        glm::vec3(4.3f, 0.5f, 21.0f),
        glm::vec3(-15.6f, 0.5f, -24.3f),
        glm::vec3(18.9f, 0.5f, 14.1f),
        glm::vec3(-7.2f, 0.5f, 19.5f),
        glm::vec3(23.4f, 0.5f, -6.8f),
        glm::vec3(-11.5f, 0.5f, 2.7f),
        glm::vec3(9.8f, 0.5f, -21.4f),
        glm::vec3(-3.4f, 0.5f, -14.6f),
        glm::vec3(16.7f, 0.5f, 5.2f),
        glm::vec3(-20.8f, 0.5f, -9.1f),
        glm::vec3(1.2f, 0.5f, 17.8f),
        glm::vec3(24.5f, 0.5f, 22.3f),
        glm::vec3(-18.3f, 0.5f, 11.6f),
        glm::vec3(6.5f, 0.5f, -12.9f),
        glm::vec3(-24.1f, 0.5f, -1.5f),
        glm::vec3(14.2f, 0.5f, 8.9f),
        glm::vec3(-9.7f, 0.5f, -17.2f),
        glm::vec3(21.6f, 0.5f, -3.4f)
    };

    auto coinModel = std::make_shared<Model>("../models/gold_coin/scene.gltf");
    for (const auto& pos : coinPositions) {
        auto coin = std::make_shared<GoldCoin>(coinModel, pos);
        
        auto coinLight = std::make_shared<PointLight>();
        coinLight->Position = pos;
        coinLight->Color = glm::vec3(1.0f, 0.84f, 0.0f); // Gold color
        coinLight->Intensity = 2.0f;
        coinLight->Radius = 5.0f;
        
        coin->m_Light = coinLight;
        
        m_Scene->AddPointLight(coinLight);
        m_Scene->AddEntity(coin);
        m_GoldCoins.push_back(coin);
    }
    
    // Add Fish
    auto turtleModel = std::make_shared<Model>("../models/fish/sea_turtle/scene.gltf");
    auto discusModel = std::make_shared<Model>("../models/fish/discus_fish/scene.gltf");
    auto lowPolyFishModel = std::make_shared<Model>("../models/fish/low_poly_fish/scene.gltf");
    
    auto turtle = std::make_shared<SeaTurtle>(turtleModel, m_Scene.get(), glm::vec3(5.0f, 2.0f, 0.0f));
    m_Scene->AddEntity(turtle);

    auto discus = std::make_shared<DiscusFish>(discusModel, m_Scene.get(), glm::vec3(-5.0f, 1.5f, 2.0f));
    auto discusLight = std::make_shared<PointLight>();
    discusLight->Position = discus->Position;
    discusLight->Color = glm::vec3(0.0f, 1.0f, 1.0f);
    discusLight->Intensity = 2.0f;
    discusLight->Radius = 3.0f;
    discus->m_Light = discusLight;
    m_Scene->AddPointLight(discusLight);
    m_Scene->AddEntity(discus);

    int numLowPolyFish = (int)m_Config->GetFloat("low_poly_fish_count", 20.0f);
    for (int i = 0; i < numLowPolyFish; ++i) {
        glm::vec3 randomPos(
            ((rand() % 100) / 100.0f) * 20.0f - 10.0f,
            ((rand() % 100) / 100.0f) * 10.0f + 1.0f,
            ((rand() % 100) / 100.0f) * 20.0f - 10.0f
        );
        auto fish = std::make_shared<FlockingFish>(lowPolyFishModel, m_Scene.get(), &m_Flock, randomPos);
        m_Scene->AddEntity(fish);
        m_Flock.push_back(fish);
    }
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    m_IsRunning = true;
}

void Application::ProcessInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    if (glfwGetKey(m_Window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!m_CKeyPressed) {
            m_PlayerCollisionEnabled = !m_PlayerCollisionEnabled;
            m_CKeyPressed = true;
            std::cout << "Player collision " << (m_PlayerCollisionEnabled ? "ENABLED" : "DISABLED") << std::endl;
        }
    } else {
        m_CKeyPressed = false;
    }

    if (glfwGetKey(m_Window, GLFW_KEY_H) == GLFW_PRESS) {
        if (!m_HKeyPressed) {
            m_HintEnabled = !m_HintEnabled;
            m_HKeyPressed = true;
        }
    } else {
        m_HKeyPressed = false;
    }

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

    if (m_PlayerCollisionEnabled) {
        m_Camera->Position.x = targetPos.x;
        m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);
        
        m_Camera->Position.y = targetPos.y;
        m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);
        
        m_Camera->Position.z = targetPos.z;
        m_Scene->CheckCollisionSphere(m_Camera->Position, cameraRadius);
    } else {
        m_Camera->Position = targetPos;
    }

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
        m_Scene->PlayerPosition = m_Camera->Position;
        m_Scene->Update(m_DeltaTime);
    }

    // Coin Collection Check
    for (auto& coin : m_GoldCoins) {
        if (coin->IsActive) {
            float dist = glm::distance(m_Camera->Position, coin->Position);
            if (dist < 1.0f) {
                coin->IsActive = false;
                m_CoinsCollected++;
                std::cout << "Gold Coin collected! Total: " << m_CoinsCollected << " / " << m_GoldCoins.size() << std::endl;
            }
        }
    }
}

void Application::Render()
{
    m_Renderer->Draw(*m_Scene, *m_Camera, m_Width, m_Height, m_Config.get());

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render HUD
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("HUD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("Position: X: %.2f Y: %.2f Z: %.2f", m_Camera->Position.x, m_Camera->Position.y, m_Camera->Position.z);
        ImGui::Text("Gold Coins: %d / %d", m_CoinsCollected, (int)m_GoldCoins.size());
        if (m_PlayerCollisionEnabled)
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Collision: ENABLED");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Collision: DISABLED");
        ImGui::Text("Press 'C' to toggle collision");
        
        if (m_HintEnabled)
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Hint: ENABLED");
        else
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Hint: DISABLED (Press 'H')");
    }
    ImGui::End();

    if (m_HintEnabled) {
        std::shared_ptr<GoldCoin> closestCoin = nullptr;
        float minDist = 999999.0f;
        for (const auto& coin : m_GoldCoins) {
            if (coin->IsActive) {
                float dist = glm::distance(m_Camera->Position, coin->Position);
                if (dist < minDist) {
                    minDist = dist;
                    closestCoin = coin;
                }
            }
        }
        
        if (closestCoin) {
            glm::vec3 dirToCoin = glm::normalize(closestCoin->Position - m_Camera->Position);
            float dotFront = glm::dot(m_Camera->Front, dirToCoin);
            float dotRight = glm::dot(m_Camera->Right, dirToCoin);
            
            ImVec2 screenSize = ImGui::GetIO().DisplaySize;
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            ImU32 redColor = IM_COL32(255, 0, 0, 100);
            ImU32 transColor = IM_COL32(255, 0, 0, 0);
            float thickness = 100.0f;
            
            if (dotFront > 0.5f) { // Front
                drawList->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(screenSize.x, thickness), redColor, redColor, transColor, transColor);
            } else if (dotFront < -0.5f) { // Back
                drawList->AddRectFilledMultiColor(ImVec2(0, screenSize.y - thickness), ImVec2(screenSize.x, screenSize.y), transColor, transColor, redColor, redColor);
            }
            
            if (dotRight > 0.5f) { // Right
                drawList->AddRectFilledMultiColor(ImVec2(screenSize.x - thickness, 0), ImVec2(screenSize.x, screenSize.y), transColor, redColor, redColor, transColor);
            } else if (dotRight < -0.5f) { // Left
                drawList->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(thickness, screenSize.y), redColor, transColor, transColor, redColor);
            }
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_Window);
}

void Application::Shutdown()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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
