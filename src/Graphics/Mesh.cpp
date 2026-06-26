#include "Mesh.h"
#include <cstddef>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    
    if (!vertices.empty()) {
        localAABB.Min = vertices[0].Position;
        localAABB.Max = vertices[0].Position;
        for (const auto& v : vertices) {
            localAABB.Min = glm::min(localAABB.Min, v.Position);
            localAABB.Max = glm::max(localAABB.Max, v.Position);
        }
    } else {
        localAABB.Min = glm::vec3(0.0f);
        localAABB.Max = glm::vec3(0.0f);
    }

    setupMesh();
    buildBVH();
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader) const 
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int metallicRoughnessNr = 1;
    
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;

    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); 
        std::string number;
        std::string name = textures[i]->Type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);
        else if(name == "texture_normal") {
            number = std::to_string(normalNr++);
            hasNormalMap = true;
        } else if(name == "texture_metallicRoughness") {
            number = std::to_string(metallicRoughnessNr++);
            hasMetallicRoughnessMap = true;
        }
        
        glUniform1i(glGetUniformLocation(shader.GetID(), (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->ID);
    }
    
    shader.SetInt("hasNormalMap", hasNormalMap ? 1 : 0);
    shader.SetInt("hasMetallicRoughnessMap", hasMetallicRoughnessMap ? 1 : 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::buildBVH() {
    int numTris = indices.size() / 3;
    if (numTris == 0) return;
    
    bvhTriIndices.resize(numTris);
    for(int i=0; i<numTris; ++i) bvhTriIndices[i] = i;

    BVHNode root;
    root.firstTri = 0;
    root.triCount = numTris;
    bvhNodes.push_back(root);
    
    updateNodeBounds(0);
    subdivide(0);
}

void Mesh::updateNodeBounds(int nodeIdx) {
    BVHNode& node = bvhNodes[nodeIdx];
    node.bounds.Min = glm::vec3(1e30f);
    node.bounds.Max = glm::vec3(-1e30f);
    for(int i = 0; i < node.triCount; ++i) {
        int triIdx = bvhTriIndices[node.firstTri + i];
        glm::vec3 v0 = vertices[indices[triIdx*3+0]].Position;
        glm::vec3 v1 = vertices[indices[triIdx*3+1]].Position;
        glm::vec3 v2 = vertices[indices[triIdx*3+2]].Position;
        node.bounds.Min = glm::min(node.bounds.Min, glm::min(v0, glm::min(v1, v2)));
        node.bounds.Max = glm::max(node.bounds.Max, glm::max(v0, glm::max(v1, v2)));
    }
}

void Mesh::subdivide(int nodeIdx) {
    BVHNode& node = bvhNodes[nodeIdx];
    if (node.triCount <= 4) return; // leaf

    glm::vec3 extent = node.bounds.Max - node.bounds.Min;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;
    float splitPos = node.bounds.Min[axis] + extent[axis] * 0.5f;

    int i = node.firstTri;
    int j = i + node.triCount - 1;
    while (i <= j) {
        int triIdx = bvhTriIndices[i];
        glm::vec3 v0 = vertices[indices[triIdx*3+0]].Position;
        glm::vec3 v1 = vertices[indices[triIdx*3+1]].Position;
        glm::vec3 v2 = vertices[indices[triIdx*3+2]].Position;
        float center = (v0[axis] + v1[axis] + v2[axis]) / 3.0f;
        if (center < splitPos) {
            i++;
        } else {
            std::swap(bvhTriIndices[i], bvhTriIndices[j]);
            j--;
        }
    }
    
    int leftCount = i - node.firstTri;
    if (leftCount == 0 || leftCount == node.triCount) return;

    int leftChildIdx = bvhNodes.size();
    bvhNodes.push_back(BVHNode());
    int rightChildIdx = bvhNodes.size();
    bvhNodes.push_back(BVHNode());

    // Update references carefully as pushing back might reallocate
    bvhNodes[leftChildIdx].firstTri = bvhNodes[nodeIdx].firstTri;
    bvhNodes[leftChildIdx].triCount = leftCount;
    bvhNodes[rightChildIdx].firstTri = i;
    bvhNodes[rightChildIdx].triCount = bvhNodes[nodeIdx].triCount - leftCount;

    bvhNodes[nodeIdx].left = leftChildIdx;
    bvhNodes[nodeIdx].right = rightChildIdx;
    bvhNodes[nodeIdx].triCount = 0;

    updateNodeBounds(leftChildIdx);
    updateNodeBounds(rightChildIdx);
    
    subdivide(leftChildIdx);
    subdivide(rightChildIdx);
}

void Mesh::CheckCollision(glm::vec3& localSphereCenter, float localRadius) const {
    if (bvhNodes.empty()) return;
    checkCollisionBVH(0, localSphereCenter, localRadius);
}

void Mesh::checkCollisionBVH(int nodeIdx, glm::vec3& localSphereCenter, float localRadius) const {
    const BVHNode& node = bvhNodes[nodeIdx];
    if (!CheckCollisionAABBSphere(node.bounds, localSphereCenter, localRadius)) return;

    if (node.triCount > 0) {
        for (int i = 0; i < node.triCount; ++i) {
            int triIdx = bvhTriIndices[node.firstTri + i];
            glm::vec3 v0 = vertices[indices[triIdx*3+0]].Position;
            glm::vec3 v1 = vertices[indices[triIdx*3+1]].Position;
            glm::vec3 v2 = vertices[indices[triIdx*3+2]].Position;
            
            glm::vec3 pen;
            if (CheckCollisionSphereTriangle(localSphereCenter, localRadius, v0, v1, v2, pen)) {
                localSphereCenter += pen;
            }
        }
    } else {
        if (node.left != -1) checkCollisionBVH(node.left, localSphereCenter, localRadius);
        if (node.right != -1) checkCollisionBVH(node.right, localSphereCenter, localRadius);
    }
}
