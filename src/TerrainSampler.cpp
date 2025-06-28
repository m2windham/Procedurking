#include "TerrainSampler.h"
#include "Icosphere.h"
#include <algorithm>
#include <limits>

TerrainSampler::TerrainSampler(const Icosphere* icosphere) : terrain(icosphere)
{
    buildTriangleCache();
}

float TerrainSampler::GetHeightAtPosition(const glm::vec3& worldPos) const
{
    // Project the world position to the unit sphere
    glm::vec3 sphereDir = glm::normalize(worldPos);
    
    // Find the triangle that contains this direction
    const Triangle* bestTriangle = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    
    for (const auto& triangle : triangles) {
        // Calculate triangle center direction
        glm::vec3 triCenter = glm::normalize((triangle.v0_sphere + triangle.v1_sphere + triangle.v2_sphere) / 3.0f);
        
        // Check if point is close to this triangle
        float distance = glm::length(sphereDir - triCenter);
        
        if (distance < bestDistance) {
            // More precise check: is the point actually inside this triangle when projected?
            if (isPointInSphericalTriangle(sphereDir, triangle)) {
                bestTriangle = &triangle;
                bestDistance = distance;
            } else if (distance < bestDistance) {
                // Keep as backup if no perfect match found
                bestTriangle = &triangle;
                bestDistance = distance;
            }
        }
    }
    
    if (bestTriangle) {
        return interpolateHeightFromTriangle(*bestTriangle, sphereDir);
    }
    
    // Fallback: return base radius
    return 1.0f;
}

glm::vec3 TerrainSampler::GetNormalAtPosition(const glm::vec3& worldPos) const
{
    // Project the world position to the unit sphere
    glm::vec3 sphereDir = glm::normalize(worldPos);
    
    // Find the closest triangle
    const Triangle* bestTriangle = nullptr;
    float bestDistance = std::numeric_limits<float>::max();
    
    for (const auto& triangle : triangles) {
        glm::vec3 triCenter = glm::normalize((triangle.v0_sphere + triangle.v1_sphere + triangle.v2_sphere) / 3.0f);
        float distance = glm::length(sphereDir - triCenter);
        
        if (distance < bestDistance) {
            bestTriangle = &triangle;
            bestDistance = distance;
        }
    }
    
    if (bestTriangle) {
        // Calculate the normal from the actual displaced triangle
        glm::vec3 edge1 = bestTriangle->v1_world - bestTriangle->v0_world;
        glm::vec3 edge2 = bestTriangle->v2_world - bestTriangle->v0_world;
        return glm::normalize(glm::cross(edge1, edge2));
    }
    
    // Fallback: return sphere normal
    return sphereDir;
}

void TerrainSampler::buildTriangleCache()
{
    const auto& vertices = terrain->getVertices();
    const auto& indices = terrain->getIndices();
    const auto& elevations = terrain->getElevations();
    
    triangles.clear();
    triangles.reserve(indices.size() / 3);
    
    for (size_t i = 0; i < indices.size(); i += 3) {
        Triangle tri;
        
        // Store both the sphere-normalized positions and world positions
        tri.v0_world = vertices[indices[i]];
        tri.v1_world = vertices[indices[i + 1]];
        tri.v2_world = vertices[indices[i + 2]];
        
        tri.v0_sphere = glm::normalize(tri.v0_world);
        tri.v1_sphere = glm::normalize(tri.v1_world);
        tri.v2_sphere = glm::normalize(tri.v2_world);
        
        tri.e0 = elevations[indices[i]];
        tri.e1 = elevations[indices[i + 1]];
        tri.e2 = elevations[indices[i + 2]];
        
        triangles.push_back(tri);
    }
}

float TerrainSampler::interpolateHeightFromTriangle(const Triangle& tri, const glm::vec3& sphereDir) const
{
    // Use barycentric coordinates on the sphere triangle
    glm::vec3 baryCoords = calculateBarycentricCoords(sphereDir, tri.v0_sphere, tri.v1_sphere, tri.v2_sphere);
    
    // Interpolate the world positions using barycentric coordinates
    glm::vec3 interpolatedWorld = baryCoords.x * tri.v0_world + 
                                  baryCoords.y * tri.v1_world + 
                                  baryCoords.z * tri.v2_world;
    
    // Return the distance from origin (which gives us the radius + elevation)
    return glm::length(interpolatedWorld);
}

glm::vec3 TerrainSampler::calculateBarycentricCoords(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) const
{
    // Calculate barycentric coordinates for a point on a spherical triangle
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 v2 = p - a;
    
    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);
    
    float denom = dot00 * dot11 - dot01 * dot01;
    if (std::abs(denom) < 1e-6f) {
        // Degenerate triangle, return equal weights
        return glm::vec3(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
    }
    
    float invDenom = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w = 1.0f - u - v;
    
    // Clamp to valid barycentric coordinates
    if (u < 0.0f || v < 0.0f || w < 0.0f || u + v > 1.0f) {
        // Point is outside triangle, project to closest edge/vertex
        if (u < 0.0f) {
            u = 0.0f;
            v = std::max(0.0f, std::min(1.0f, glm::dot(v2, v1) / glm::dot(v1, v1)));
            w = 1.0f - v;
        } else if (v < 0.0f) {
            v = 0.0f;
            u = std::max(0.0f, std::min(1.0f, glm::dot(v2, v0) / glm::dot(v0, v0)));
            w = 1.0f - u;
        } else if (w < 0.0f) {
            w = 0.0f;
            float total = u + v;
            if (total > 1.0f) {
                u /= total;
                v /= total;
            }
        }
    }
    
    return glm::vec3(w, u, v); // Note: w, u, v order to match a, b, c
}

bool TerrainSampler::isPointInSphericalTriangle(const glm::vec3& p, const Triangle& tri) const
{
    // Check if point p is inside the spherical triangle using barycentric coordinates
    glm::vec3 baryCoords = calculateBarycentricCoords(p, tri.v0_sphere, tri.v1_sphere, tri.v2_sphere);
    
    // Point is inside if all barycentric coordinates are non-negative and sum to 1
    return baryCoords.x >= -1e-6f && baryCoords.y >= -1e-6f && baryCoords.z >= -1e-6f &&
           std::abs(baryCoords.x + baryCoords.y + baryCoords.z - 1.0f) < 1e-6f;
}

float TerrainSampler::interpolateHeight(const Triangle& tri, const glm::vec3& point) const
{
    // Legacy method - now redirects to the improved version
    return interpolateHeightFromTriangle(tri, glm::normalize(point));
}

bool TerrainSampler::isPointInTriangle(const glm::vec3& p, const Triangle& tri) const
{
    // Legacy method - now redirects to the improved version
    return isPointInSphericalTriangle(glm::normalize(p), tri);
}

glm::vec3 TerrainSampler::projectToSphere(const glm::vec3& point, float radius) const
{
    return glm::normalize(point) * radius;
} 