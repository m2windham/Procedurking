#pragma once

#include <glm/glm.hpp>
#include <vector>

class Icosphere;

class TerrainSampler
{
public:
    TerrainSampler(const Icosphere* icosphere);
    
    float GetHeightAtPosition(const glm::vec3& worldPos) const;
    glm::vec3 GetNormalAtPosition(const glm::vec3& worldPos) const;

private:
    const Icosphere* terrain;
    
    struct Triangle {
        // Sphere-normalized positions for direction calculations
        glm::vec3 v0_sphere, v1_sphere, v2_sphere;
        // Actual world positions (displaced by terrain)
        glm::vec3 v0_world, v1_world, v2_world;
        // Elevation values
        float e0, e1, e2;
    };
    
    std::vector<Triangle> triangles;
    void buildTriangleCache();
    
    // New improved methods
    float interpolateHeightFromTriangle(const Triangle& tri, const glm::vec3& sphereDir) const;
    glm::vec3 calculateBarycentricCoords(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) const;
    bool isPointInSphericalTriangle(const glm::vec3& p, const Triangle& tri) const;
    
    // Legacy methods for compatibility
    float interpolateHeight(const Triangle& tri, const glm::vec3& point) const;
    bool isPointInTriangle(const glm::vec3& p, const Triangle& tri) const;
    glm::vec3 projectToSphere(const glm::vec3& point, float radius) const;
}; 