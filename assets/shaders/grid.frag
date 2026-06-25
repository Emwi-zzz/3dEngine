#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 WorldPos;

void main()
{
    gPosition = WorldPos;
    gNormal = vec3(0.0, 1.0, 0.0);
    
    vec2 grid = abs(fract(WorldPos.xz - 0.5) - 0.5) / fwidth(WorldPos.xz);
    float line = min(grid.x, grid.y);
    float color = 1.0 - min(line, 1.0);
    
    gAlbedoSpec.rgb = vec3(0.1) + vec3(0.5) * color;
    gAlbedoSpec.a = 0.0;
}
