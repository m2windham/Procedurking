#pragma once

#include <vector>
#include <glm/glm.hpp>

class FastNoiseLite;

struct TerrainConfig {
    float continentAmplitude = 0.3f;
    float continentFrequency = 0.5f;
    
    float mountainAmplitude = 0.15f;
    float mountainFrequency = 2.0f;
    
    float hillAmplitude = 0.08f;
    float hillFrequency = 4.0f;
    
    float detailAmplitude = 0.03f;
    float detailFrequency = 8.0f;
    
    float oceanLevel = 0.1f;
    float maxElevation = 0.5f;
};

class Icosphere
{
public:
    Icosphere(float radius, int subdivisions);
    void generateTerrain(const TerrainConfig& config);
    void calculateNormals();

    const std::vector<glm::vec3>& getVertices() const { return vertices; }
    const std::vector<glm::vec3>& getNormals() const { return normals; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    const std::vector<float>& getElevations() const { return elevations; }

private:
    void generateBaseIcosahedron();
    void subdivide();
    unsigned int addVertex(const glm::vec3& vertex);

    float radius;
    int subdivisions;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<float> elevations;
}; 