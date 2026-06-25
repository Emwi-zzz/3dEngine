#pragma once
#include <glad/gl.h>
#include <string>

class Texture {
public:
    Texture(const std::string& path, const std::string& typeName = "texture_diffuse");
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    std::string Type;
    std::string Path;
    GLuint ID;
};
