#version 330 core
out vec4 FragColor;

in vec4 ParticleColor;
in vec2 TexCoord;

void main()
{
    float dist = length(TexCoord);
    if (dist > 1.0) discard;
    
    float edge = pow(dist, 2.0);
    vec4 finalColor = ParticleColor;
    finalColor.a *= mix(0.2, 1.0, edge);
    
    FragColor = finalColor;
}
