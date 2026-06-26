#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;

uniform mat4 lightSpaceMatrix;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform float ambientIntensity;
uniform float lightIntensity;
uniform vec3 lightColor;

uniform float fogDensity;
uniform vec3 fogColor;

#define MAX_POINT_LIGHTS 32
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

const float PI = 3.14159265359;

// GGX/Trowbridge-Reitz Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Schlick-GGX Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{             
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec4 normalData = texture(gNormal, TexCoords);
    vec3 Normal = normalData.rgb;
    float metallic = normalData.a;
    
    vec4 albedoData = texture(gAlbedoSpec, TexCoords);
    vec3 albedo = albedoData.rgb;
    float roughness = max(albedoData.a, 0.05);
    
    if (length(Normal) < 0.1) {
        // Empty background is filled entirely with the fog color (water color)
        FragColor = vec4(fogColor, 1.0);
        return;
    }

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    // Reflectance equation
    vec3 Lo = vec3(0.0);
    
    // 1. Directional Light
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor * lightIntensity;
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    float NdotL = max(dot(N, L), 0.0);        
    // Shadow Calculation
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float shadow = 0.0;
    if(projCoords.z <= 1.0) {
        float closestDepth = texture(shadowMap, projCoords.xy).r; 
        float currentDepth = projCoords.z;
        float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);  
        
        // PCF
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
            }    
        }
        shadow /= 9.0;
    }
    
    Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL;

    // 2. Point lights
    for (int i = 0; i < numPointLights; ++i) {
        vec3 plDir = pointLights[i].position - FragPos;
        float dist = length(plDir);
        if (dist < pointLights[i].radius) {
            vec3 pL = normalize(plDir);
            vec3 pH = normalize(V + pL);
            float attenuation = 1.0 - (dist / pointLights[i].radius);
            attenuation = attenuation * attenuation; 
            vec3 pRadiance = pointLights[i].color * pointLights[i].intensity * attenuation;
            
            float pNDF = DistributionGGX(N, pH, roughness);   
            float pG   = GeometrySmith(N, V, pL, roughness);      
            vec3 pF    = fresnelSchlick(max(dot(pH, V), 0.0), F0);
               
            vec3 pNumerator    = pNDF * pG * pF; 
            float pDenominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, pL), 0.0) + 0.0001;
            vec3 pSpecular = pNumerator / pDenominator;
            
            vec3 pkS = pF;
            vec3 pkD = vec3(1.0) - pkS;
            pkD *= 1.0 - metallic;	  
            
            float pNdotL = max(dot(N, pL), 0.0);        
            
            // Subsurface/Glow hack: illuminate object from inside if light is very close
            float innerGlow = smoothstep(2.0, 0.0, dist) * 0.1;
            pNdotL = max(pNdotL, innerGlow);

            Lo += (pkD * albedo / PI + pSpecular) * pRadiance * pNdotL;
        }
    }

    vec3 ambient = vec3(0.03) * albedo * ambientIntensity;
    vec3 lighting = ambient + Lo;
    
    // HDR tonemapping
    lighting = lighting / (lighting + vec3(1.0));
    // gamma correct
    lighting = pow(lighting, vec3(1.0/2.2)); 

    // --- DEPTH-BASED FOG ---
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-pow((distance * fogDensity), 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 finalColor = mix(fogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
