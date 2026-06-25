#pragma once
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 Min;
    glm::vec3 Max;
};

inline bool CheckCollisionAABB(const AABB& a, const AABB& b) {
    return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
           (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
           (a.Min.z <= b.Max.z && a.Max.z >= b.Min.z);
}

inline bool CheckCollisionAABBSphere(const AABB& aabb, const glm::vec3& center, float radius) {
    glm::vec3 closest = glm::clamp(center, aabb.Min, aabb.Max);
    glm::vec3 v = center - closest;
    return glm::dot(v, v) <= (radius * radius);
}

inline glm::vec3 ClosestPointOnTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ap = p - a;

    float d1 = glm::dot(ab, ap);
    float d2 = glm::dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f) return a;

    glm::vec3 bp = p - b;
    float d3 = glm::dot(ab, bp);
    float d4 = glm::dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3) return b;

    float vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
        float v = d1 / (d1 - d3);
        return a + v * ab;
    }

    glm::vec3 cp = p - c;
    float d5 = glm::dot(ab, cp);
    float d6 = glm::dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return c;

    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return a + w * ac;
    }

    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + w * (c - b);
    }

    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;
    return a + ab * v + ac * w;
}

inline bool CheckCollisionSphereTriangle(const glm::vec3& center, float radius, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, glm::vec3& outPenetration) {
    glm::vec3 closest = ClosestPointOnTriangle(center, a, b, c);
    glm::vec3 v = center - closest;
    float distSq = glm::dot(v, v);
    if (distSq < radius * radius) {
        if (distSq < 0.000001f) {
            glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
            outPenetration = normal * radius;
            return true;
        }
        float dist = std::sqrt(distSq);
        outPenetration = (v / dist) * (radius - dist);
        return true;
    }
    return false;
}

struct BVHNode {
    AABB bounds;
    int left = -1;
    int right = -1;
    int firstTri = -1;
    int triCount = 0;
};
