#pragma once
#include <string>
#include <map>
#include <glm/glm.hpp>
#include "Graphics/Shader.h"

struct Character {
    unsigned int TextureID; 
    glm::ivec2   Size;      
    glm::ivec2   Bearing;   
    unsigned int Advance;   
};

class TextRenderer {
public:
    TextRenderer(unsigned int width, unsigned int height);
    ~TextRenderer();

    void Load(const std::string& font, unsigned int fontSize);
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
    void Resize(unsigned int width, unsigned int height);

private:
    std::map<char, Character> Characters;
    Shader* TextShader;
    unsigned int VAO, VBO;
    unsigned int m_Width, m_Height;
};
