#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in mat4 aInstanceMatrix;
layout (location = 5) in vec4 aInstanceColor;

uniform mat4 projection;
uniform mat4 view;

out vec4 ParticleColor;
out vec2 TexCoord;

void main()
{
    ParticleColor = aInstanceColor;
    TexCoord = aPos.xy * 2.0;
    
    mat4 modelView = view * aInstanceMatrix;
    float sx = length(vec3(aInstanceMatrix[0]));
    float sy = length(vec3(aInstanceMatrix[1]));
    float sz = length(vec3(aInstanceMatrix[2]));
    
    modelView[0][0] = sx; modelView[0][1] = 0.0; modelView[0][2] = 0.0;
    modelView[1][0] = 0.0; modelView[1][1] = sy; modelView[1][2] = 0.0;
    modelView[2][0] = 0.0; modelView[2][1] = 0.0; modelView[2][2] = sz;

    gl_Position = projection * modelView * vec4(aPos, 1.0);
}
