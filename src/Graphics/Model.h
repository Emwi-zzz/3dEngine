#pragma once
#include "Graphics/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <map>

class Model 
{
public:
    AABB localAABB;
    Model(const std::string& path);
    void Draw(const Shader& shader) const;
    void CheckCollision(glm::vec3& localSphereCenter, float localRadius) const;

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::map<std::string, std::shared_ptr<Texture>> textures_loaded; 

    void loadModel(const std::string& path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
