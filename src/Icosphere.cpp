#include "Icosphere.h"
#include <Cpp/FastNoiseLite.h>
#include <cmath>
#include <map>
#include <algorithm>

// Helper function for smooth interpolation
float smoothstep(float edge0, float edge1, float x) {
    float t = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0f - 2.0f * t);
}

Icosphere::Icosphere(float radius, int subdivisions) : radius(radius), subdivisions(subdivisions)
{
    generateBaseIcosahedron();
    for (int i = 0; i < subdivisions; ++i)
    {
        subdivide();
    }
}

void Icosphere::generateTerrain(const TerrainConfig& config)
{
    elevations.clear();
    elevations.reserve(vertices.size());

    // Create noise generators with proper Earth-like parameters
    FastNoiseLite continentNoise;
    continentNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    continentNoise.SetFrequency(0.8f); // Large continent features
    continentNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    continentNoise.SetFractalOctaves(4);

    FastNoiseLite oceanNoise;
    oceanNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    oceanNoise.SetFrequency(1.2f); // Ocean basin features
    oceanNoise.SetFractalOctaves(3);

    FastNoiseLite mountainNoise;
    mountainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    mountainNoise.SetFrequency(3.0f);
    mountainNoise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    mountainNoise.SetFractalOctaves(5);

    FastNoiseLite hillNoise;
    hillNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    hillNoise.SetFrequency(6.0f);
    hillNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    hillNoise.SetFractalOctaves(3);

    FastNoiseLite detailNoise;
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    detailNoise.SetFrequency(12.0f);
    detailNoise.SetFractalOctaves(2);

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        glm::vec3 spherePos = glm::normalize(vertices[i]);
        
        float x = spherePos.x;
        float y = spherePos.y;
        float z = spherePos.z;

        // Generate continent mask - this determines land vs ocean
        float continentValue = continentNoise.GetNoise(x, y, z);
        float oceanValue = oceanNoise.GetNoise(x * 0.5f, y * 0.5f, z * 0.5f);
        
        // Combine continent and ocean noise for realistic distribution
        float landMask = (continentValue * 0.7f + oceanValue * 0.3f + 0.1f);
        
        // Create realistic Earth-like land/ocean ratio (about 30% land, 70% ocean)
        bool isLand = landMask > 0.1f;
        
        float elevation = 0.0f;
        
        if (isLand) {
            // LAND GENERATION
            
            // Base land elevation
            float landHeight = (landMask - 0.1f) / 0.9f; // Normalize land portion
            elevation = landHeight * 0.05f; // Base land height above sea level
            
            // Add mountains
            float mountainValue = mountainNoise.GetNoise(x * 2.0f, y * 2.0f, z * 2.0f);
            mountainValue = std::abs(mountainValue); // Ridged mountains
            if (landHeight > 0.3f) { // Mountains only on stable continental areas
                float mountainMask = smoothstep(0.3f, 0.8f, landHeight);
                elevation += mountainValue * 0.3f * mountainMask;
            }
            
            // Add hills
            float hillValue = hillNoise.GetNoise(x * 3.0f, y * 3.0f, z * 3.0f);
            hillValue = (hillValue + 1.0f) / 2.0f;
            if (landHeight > 0.2f && landHeight < 0.7f) {
                float hillMask = smoothstep(0.2f, 0.7f, landHeight);
                elevation += hillValue * 0.1f * hillMask;
            }
            
            // Add surface details
            float detailValue = detailNoise.GetNoise(x * 8.0f, y * 8.0f, z * 8.0f);
            detailValue = (detailValue + 1.0f) / 2.0f;
            elevation += detailValue * 0.02f;
            
            // Ensure minimum land elevation
            elevation = std::max(elevation, 0.005f);
            
        } else {
            // OCEAN GENERATION
            
            // Ocean depth based on distance from land
            float oceanDepth = (0.1f - landMask) / 0.1f;
            oceanDepth = std::max(0.0f, std::min(1.0f, oceanDepth));
            
            // Ocean floor elevation (negative)
            elevation = -0.02f - (oceanDepth * 0.08f); // Ocean depths from -0.02 to -0.1
            
            // Ocean floor features
            float oceanFloorDetail = detailNoise.GetNoise(x * 4.0f, y * 4.0f, z * 4.0f);
            elevation += oceanFloorDetail * 0.01f;
            
            // Continental shelf - shallower near land
            if (oceanDepth < 0.3f) {
                elevation += (0.3f - oceanDepth) * 0.03f; // Shallow continental shelf
            }
        }
        
        // Clamp final elevation
        elevation = std::max(elevation, -0.15f);
        elevation = std::min(elevation, 0.5f);
        
        // Displace vertex
        vertices[i] = spherePos * (radius + elevation);
        elevations.push_back(elevation);
    }
    
    calculateNormals();
}

void Icosphere::calculateNormals()
{
    normals.clear();
    normals.resize(vertices.size(), glm::vec3(0.0f));
    
    // Calculate face normals and accumulate to vertices
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        glm::vec3 v0 = vertices[indices[i]];
        glm::vec3 v1 = vertices[indices[i + 1]];
        glm::vec3 v2 = vertices[indices[i + 2]];
        
        glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        
        normals[indices[i]] += faceNormal;
        normals[indices[i + 1]] += faceNormal;
        normals[indices[i + 2]] += faceNormal;
    }
    
    // Normalize all vertex normals
    for (auto& normal : normals)
    {
        normal = glm::normalize(normal);
    }
}

void Icosphere::generateBaseIcosahedron()
{
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    addVertex(glm::normalize(glm::vec3(-1,  t,  0)));
    addVertex(glm::normalize(glm::vec3( 1,  t,  0)));
    addVertex(glm::normalize(glm::vec3(-1, -t,  0)));
    addVertex(glm::normalize(glm::vec3( 1, -t,  0)));

    addVertex(glm::normalize(glm::vec3( 0, -1,  t)));
    addVertex(glm::normalize(glm::vec3( 0,  1,  t)));
    addVertex(glm::normalize(glm::vec3( 0, -1, -t)));
    addVertex(glm::normalize(glm::vec3( 0,  1, -t)));

    addVertex(glm::normalize(glm::vec3( t,  0, -1)));
    addVertex(glm::normalize(glm::vec3( t,  0,  1)));
    addVertex(glm::normalize(glm::vec3(-t,  0, -1)));
    addVertex(glm::normalize(glm::vec3(-t,  0,  1)));

    indices = {
        0, 11, 5,  0, 5, 1,   0, 1, 7,    0, 7, 10,   0, 10, 11,
        1, 5, 9,   5, 11, 4,  11, 10, 2,  10, 7, 6,   7, 1, 8,
        3, 9, 4,   3, 4, 2,   3, 2, 6,    3, 6, 8,    3, 8, 9,
        4, 9, 5,   2, 4, 11,  6, 2, 10,   8, 6, 7,    9, 8, 1
    };
}

void Icosphere::subdivide()
{
    std::vector<unsigned int> newIndices;
    std::map<std::pair<unsigned int, unsigned int>, unsigned int> midpointCache;

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int v1_idx = indices[i];
        unsigned int v2_idx = indices[i + 1];
        unsigned int v3_idx = indices[i + 2];

        auto getMidpoint = [&](unsigned int p1_idx, unsigned int p2_idx)
        {
            bool firstIsSmaller = p1_idx < p2_idx;
            unsigned int smallerIndex = firstIsSmaller ? p1_idx : p2_idx;
            unsigned int greaterIndex = firstIsSmaller ? p2_idx : p1_idx;
            auto key = std::make_pair(smallerIndex, greaterIndex);

            auto it = midpointCache.find(key);
            if (it != midpointCache.end())
            {
                return it->second;
            }

            glm::vec3 p1 = vertices[p1_idx];
            glm::vec3 p2 = vertices[p2_idx];
            glm::vec3 middle = glm::normalize(p1 + p2);
            
            unsigned int mid_idx = addVertex(middle);
            midpointCache[key] = mid_idx;
            return mid_idx;
        };

        unsigned int m12 = getMidpoint(v1_idx, v2_idx);
        unsigned int m23 = getMidpoint(v2_idx, v3_idx);
        unsigned int m31 = getMidpoint(v3_idx, v1_idx);

        newIndices.push_back(v1_idx); newIndices.push_back(m12); newIndices.push_back(m31);
        newIndices.push_back(v2_idx); newIndices.push_back(m23); newIndices.push_back(m12);
        newIndices.push_back(v3_idx); newIndices.push_back(m31); newIndices.push_back(m23);
        newIndices.push_back(m12);    newIndices.push_back(m23); newIndices.push_back(m31);
    }
    indices = newIndices;
}

unsigned int Icosphere::addVertex(const glm::vec3& vertex)
{
    vertices.push_back(glm::normalize(vertex) * radius);
    return (unsigned int)vertices.size() - 1;
} 