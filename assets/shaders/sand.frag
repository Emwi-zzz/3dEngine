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
    
    // Sand color variations based on height or noise
    vec3 baseColor = vec3(0.82, 0.73, 0.55);
    vec3 darkColor = vec3(0.70, 0.60, 0.40);
    
    // Simple variation based on WorldPos to make it look less uniform
    float noise = fract(sin(dot(WorldPos.xz, vec2(12.9898, 78.233))) * 43758.5453);
    vec3 finalColor = mix(darkColor, baseColor, noise * 0.2 + 0.8);

    gAlbedoSpec.rgb = finalColor;
    gAlbedoSpec.a = 0.0;
}
