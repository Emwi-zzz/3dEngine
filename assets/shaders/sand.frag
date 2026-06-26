#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 WorldPos;
in vec3 Normal;

void main()
{
    gPosition = WorldPos;
    gNormal = normalize(Normal);
    
    // Warm sand color
    vec3 sandColor = vec3(0.76, 0.70, 0.50);
    
    // Add subtle variation based on height
    float heightVar = smoothstep(0.0, 5.0, WorldPos.y);
    sandColor = mix(sandColor, vec3(0.80, 0.75, 0.55), heightVar);
    
    gAlbedoSpec.rgb = sandColor;
    gAlbedoSpec.a = 0.0; // Rough material, no specularity
}
