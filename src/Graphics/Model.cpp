#include "Model.h"
#include <iostream>

Model::Model(const std::string& path)
{
    loadModel(path);
}

void Model::Draw(const Shader& shader) const
{
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
    
    if (!meshes.empty()) {
        localAABB = meshes[0].localAABB;
        for (const auto& m : meshes) {
            localAABB.Min = glm::min(localAABB.Min, m.localAABB.Min);
            localAABB.Max = glm::max(localAABB.Max, m.localAABB.Max);
        }
    } else {
        localAABB.Min = glm::vec3(0.0f);
        localAABB.Max = glm::vec3(0.0f);
    }
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        if(mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    
    std::vector<std::shared_ptr<Texture>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    
    std::vector<std::shared_ptr<Texture>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    
    std::vector<std::shared_ptr<Texture>> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    if (normalMaps.empty()) {
        normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    }
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    
    std::vector<std::shared_ptr<Texture>> metallicRoughnessMaps = loadMaterialTextures(material, aiTextureType_UNKNOWN, "texture_metallicRoughness");
    textures.insert(textures.end(), metallicRoughnessMaps.begin(), metallicRoughnessMaps.end());
    
    return Mesh(vertices, indices, textures);
}

void Model::CheckCollision(glm::vec3& localSphereCenter, float localRadius) const {
    if (!CheckCollisionAABBSphere(localAABB, localSphereCenter, localRadius)) return;

    for (const auto& mesh : meshes) {
        mesh.CheckCollision(localSphereCenter, localRadius);
    }
}

std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<std::shared_ptr<Texture>> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = directory + "/" + str.C_Str();
        
        if (textures_loaded.find(path) == textures_loaded.end())
        {
            auto texture = std::make_shared<Texture>(path, typeName);
            textures.push_back(texture);
            textures_loaded[path] = texture;
        }
        else
        {
            textures.push_back(textures_loaded[path]);
        }
    }
    return textures;
}
