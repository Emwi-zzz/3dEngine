#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 sunPosition;

const int NUM_SAMPLES = 100;
const float DENSITY = 0.2;
const float WEIGHT = 0.01;
const float DECAY = 0.98;
const float EXPOSURE = 0.8;

void main()
{
    vec2 deltaTextCoord = TexCoords - sunPosition;
    deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * DENSITY;
    
    vec2 tc = TexCoords;
    float illuminationDecay = 1.0;
    
    vec3 color = texture(screenTexture, tc).rgb;
    
    for(int i = 0; i < NUM_SAMPLES; i++)
    {
        tc -= deltaTextCoord;
        vec3 sampleColor = texture(screenTexture, tc).rgb;
        sampleColor *= illuminationDecay * WEIGHT;
        color += sampleColor;
        illuminationDecay *= DECAY;
    }
    
    FragColor = vec4(color * EXPOSURE, 1.0);
}
