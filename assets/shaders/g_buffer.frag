#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallicRoughness1;

uniform bool hasNormalMap;
uniform bool hasMetallicRoughnessMap;

void main()
{    
    gPosition = FragPos;

    vec3 finalNormal = normalize(Normal);
    if (hasNormalMap) {
        vec3 t = normalize(Tangent);
        vec3 b = normalize(Bitangent);
        vec3 n = normalize(Normal);
        mat3 tbn = mat3(t, b, n);
        
        vec3 normalMap = texture(texture_normal1, TexCoords).rgb;
        normalMap = normalMap * 2.0 - 1.0;
        finalNormal = normalize(tbn * normalMap);
    }
    
    float metallic = 0.0;
    float roughness = 0.5;
    
    if (hasMetallicRoughnessMap) {
        vec3 mr = texture(texture_metallicRoughness1, TexCoords).rgb;
        metallic = mr.b;
        roughness = mr.g;
    } else {
        float spec = texture(texture_specular1, TexCoords).r;
        roughness = 1.0 - spec; 
    }

    gNormal.rgb = finalNormal;
    gNormal.a = metallic;

    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    gAlbedoSpec.a = roughness;
}
