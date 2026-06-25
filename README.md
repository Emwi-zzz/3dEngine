# 3D Engine Documentation

Welcome to the custom 3D Engine! This guide explains how to build, run, and customize your scene with entities, models, lights, and visual effects.

## Building and Running

### Prerequisites
Make sure you have CMake and a modern C++ compiler (like GCC or Clang) installed. The engine automatically downloads its dependencies (GLFW, GLM, Assimp, Glad) via CMake FetchContent.

### Build Instructions
Open your terminal in the root directory of the project and run:

```bash
mkdir build
cd build
cmake ..
make -j4
```

### Running the Engine
After building successfully, run the executable from inside the `build` directory. The engine uses relative paths to find assets, so it's important to run it from here!

```bash
./3dEngine
```
*Note: Ensure your `config.txt` file and `models/` directory are located in the project root.*

---

## Adding Content to the Scene

All scene composition happens in `src/Core/Application.cpp` inside the `Application::Init()` function.

### 1. Adding a New Model
To load a new 3D model, create a `Model` object pointing to your `.gltf` or `.obj` file:
```cpp
auto myModel = std::make_shared<Model>("../models/my_model/scene.gltf");
```
*Because of how the engine works, you only need to load a model once, even if you want to spawn it 100 times!*

### 2. Adding an Entity
An `Entity` represents a physical object in the world. You attach a `Model` to it and set its transformations (Position, Rotation, Scale). 

```cpp
// Create the entity and attach the model
auto myEntity = std::make_shared<Entity>("MyEntity", myModel);

// Configure transformations
myEntity->Position = glm::vec3(10.0f, 0.0f, 5.0f);
myEntity->Scale = glm::vec3(0.5f); // Scale it down to 50%
myEntity->Rotation = glm::vec3(0.0f, 90.0f, 0.0f); // Rotate 90 degrees on the Y axis

// Register it with the scene
m_Scene->AddEntity(myEntity);
```
*(The physics system will automatically read the model's triangles and build a collision BVH tree for the entity!)*

### 3. Adding Lights
The engine uses Deferred Rendering, which means you can add dozens of lights with almost zero performance penalty!

```cpp
PointLight myLight;
myLight.Position = glm::vec3(5.0f, 2.0f, 0.0f);
myLight.Color = glm::vec3(1.0f, 0.0f, 0.0f); // Pure Red light
myLight.Intensity = 5.0f;
myLight.Radius = 15.0f; // How far the light reaches before fading out

m_Scene->AddPointLight(myLight);
```

### 4. Adding Particle Effects (Bubbles)
You can spawn a particle system by creating a `ParticleSystem` entity. The parameters are `particle_count`, `origin_position`, and `spawn_radius_spread`.

```cpp
// Spawns 150 bubbles around the origin within a 20-unit radius
auto bubbles = std::make_shared<ParticleSystem>(150, glm::vec3(0.0f, -1.0f, 0.0f), 20.0f);
m_Scene->AddEntity(bubbles);
```

---

## Global Configuration & Hot-Reloading

The engine features a live hot-reloading configuration system. Open `config.txt` in the root folder to tweak settings like:
- `camera_speed`
- `ambient_intensity`
- `light_intensity`
- `fog_density`
- `fog_color`

**Hot-Reloading In-Game:** While the engine is running, you can save changes to `config.txt` in your text editor and press **"R"** in the game window. The engine will instantly parse the file and apply the new graphical and gameplay settings without needing to recompile!

---

## Advanced Effects (Shaders)

If you want to tweak the advanced visual effects, you can modify the GLSL shaders directly inside the `assets/shaders/` folder.

- **God Rays (Volumetric Light Scattering):** Handled automatically in `Renderer.cpp` as a post-processing pass. You can tweak the blur density, ray length, and sample count directly in `assets/shaders/god_rays.frag`.
- **Water Fog:** Fog calculations are computed in `assets/shaders/deferred_light.frag`. The color and depth are driven by your `config.txt`.
