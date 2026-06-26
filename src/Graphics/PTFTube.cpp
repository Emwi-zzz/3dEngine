#include "PTFTube.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

PTFTube::PTFTube(const std::vector<glm::vec3>& controlPoints, float radius, int segments, int sides) : Entity("PTFTube", nullptr) {
    if (controlPoints.size() < 4) {
        std::cerr << "PTFTube requires at least 4 control points!" << std::endl;
        return;
    }

    std::vector<glm::vec3> curvePoints;
    
    // Generate spline points
    int numSplines = controlPoints.size() - 3;
    for (int i = 0; i < numSplines; ++i) {
        for (int s = 0; s < segments; ++s) {
            float t = (float)s / (float)segments;
            curvePoints.push_back(catmullRom(controlPoints[i], controlPoints[i+1], controlPoints[i+2], controlPoints[i+3], t));
        }
    }
    curvePoints.push_back(controlPoints[controlPoints.size() - 2]); // Last point

    // Parallel Transport Frames
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> binormals;

    // Initial frame
    for (size_t i = 0; i < curvePoints.size(); ++i) {
        glm::vec3 t;
        if (i == 0) t = glm::normalize(curvePoints[1] - curvePoints[0]);
        else if (i == curvePoints.size() - 1) t = glm::normalize(curvePoints[i] - curvePoints[i-1]);
        else t = glm::normalize(curvePoints[i+1] - curvePoints[i-1]);
        tangents.push_back(t);
    }

    glm::vec3 initialNormal = glm::normalize(glm::cross(tangents[0], glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glm::length(initialNormal) < 0.01f) {
        initialNormal = glm::normalize(glm::cross(tangents[0], glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    glm::vec3 initialBinormal = glm::normalize(glm::cross(tangents[0], initialNormal));

    normals.push_back(initialNormal);
    binormals.push_back(initialBinormal);

    // Transport frames
    for (size_t i = 1; i < curvePoints.size(); ++i) {
        glm::vec3 t0 = tangents[i-1];
        glm::vec3 t1 = tangents[i];
        glm::vec3 n0 = normals[i-1];
        
        glm::vec3 b = glm::cross(t0, t1);
        if (glm::length(b) < 0.0001f) {
            normals.push_back(n0);
            binormals.push_back(glm::normalize(glm::cross(t1, n0)));
            continue;
        }
        
        b = glm::normalize(b);
        float theta = acos(glm::clamp(glm::dot(t0, t1), -1.0f, 1.0f));
        glm::quat q = glm::angleAxis(theta, b);
        
        glm::vec3 n1 = glm::normalize(q * n0);
        glm::vec3 b1 = glm::normalize(glm::cross(t1, n1));
        
        normals.push_back(n1);
        binormals.push_back(b1);
    }

    std::vector<PTFVertex> vertices;
    std::vector<unsigned int> indices;

    // Generate cylinder
    for (size_t i = 0; i < curvePoints.size(); ++i) {
        glm::vec3 p = curvePoints[i];
        glm::vec3 t = tangents[i];
        glm::vec3 n = normals[i];
        glm::vec3 b = binormals[i];

        for (int s = 0; s <= sides; ++s) {
            float angle = ((float)s / sides) * glm::pi<float>() * 2.0f;
            float cosA = cos(angle);
            float sinA = sin(angle);

            glm::vec3 normal = glm::normalize(n * cosA + b * sinA);
            glm::vec3 pos = p + normal * radius;
            
            PTFVertex v;
            v.Position = pos;
            v.Normal = normal;
            v.TexCoords = glm::vec2((float)s / sides, (float)i / segments);
            v.Tangent = t;
            v.Bitangent = glm::normalize(glm::cross(normal, t));
            
            vertices.push_back(v);
        }
    }

    for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
        for (int s = 0; s < sides; ++s) {
            int current = i * (sides + 1) + s;
            int next = current + sides + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    setupMesh(vertices, indices);
}

PTFTube::~PTFTube() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void PTFTube::setupMesh(const std::vector<PTFVertex>& vertices, const std::vector<unsigned int>& indices) {
    indexCount = indices.size();
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PTFVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PTFVertex), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PTFVertex), (void*)offsetof(PTFVertex, Normal));
    // TexCoords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PTFVertex), (void*)offsetof(PTFVertex, TexCoords));
    // Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PTFVertex), (void*)offsetof(PTFVertex, Tangent));
    // Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(PTFVertex), (void*)offsetof(PTFVertex, Bitangent));

    glBindVertexArray(0);
}

void PTFTube::Draw(const Shader& shader) const {
    shader.SetMat4("model", GetModelMatrix());
    shader.SetInt("hasNormalMap", 0);
    shader.SetInt("hasMetallicRoughnessMap", 0);
    
    // Bind a simple black/grey texture just in case to texture_diffuse1
    // For now we just rely on PBR defaults in the shader for Albedo
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::vec3 PTFTube::catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    glm::vec3 v0 = (p2 - p0) * 0.5f;
    glm::vec3 v1 = (p3 - p1) * 0.5f;

    return (2.0f * p1 - 2.0f * p2 + v0 + v1) * t3 +
           (-3.0f * p1 + 3.0f * p2 - 2.0f * v0 - v1) * t2 +
           v0 * t +
           p1;
}
