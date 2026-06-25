#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

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

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    if (length(Normal) < 0.1) {
        // Empty background is filled entirely with the fog color (water color)
        FragColor = vec4(fogColor, 1.0);
        return;
    }

    // then calculate lighting as usual
    vec3 ambient = Diffuse * ambientIntensity;
    vec3 viewDir  = normalize(viewPos - FragPos);
    
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDirN = normalize(-lightDir); // directional light
    float diff = max(dot(norm, lightDirN), 0.0);
    vec3 diffuse = diff * Diffuse * lightColor * lightIntensity;
    
    // specular
    vec3 halfwayDir = normalize(lightDirN + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = vec3(0.3) * spec * Specular * lightColor * lightIntensity; 
    
    vec3 lighting = ambient + diffuse + specular;

    // Point lights
    for (int i = 0; i < numPointLights; ++i) {
        vec3 plDir = pointLights[i].position - FragPos;
        float dist = length(plDir);
        if (dist < pointLights[i].radius) {
            plDir = normalize(plDir);
            float plDiff = max(dot(norm, plDir), 0.0);
            vec3 plDiffuse = plDiff * Diffuse * pointLights[i].color * pointLights[i].intensity;
            
            vec3 plHalfway = normalize(plDir + viewDir);
            float plSpec = pow(max(dot(norm, plHalfway), 0.0), 16.0);
            vec3 plSpecular = vec3(0.3) * plSpec * Specular * pointLights[i].color * pointLights[i].intensity;
            
            float attenuation = 1.0 - (dist / pointLights[i].radius);
            attenuation = attenuation * attenuation; 
            
            lighting += (plDiffuse + plSpecular) * attenuation;
        }
    }

    // --- DEPTH-BASED FOG ---
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-pow((distance * fogDensity), 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 finalColor = mix(fogColor, lighting, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
