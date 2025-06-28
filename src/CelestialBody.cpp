#include "CelestialBody.h"
#include <glad/glad.h>
#include <cmath>
#include <Cpp/FastNoiseLite.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CelestialBody::CelestialBody(float r, int subdivisions, const OrbitalParams& orbit)
    : mesh(r, subdivisions), orbital(orbit), radius(r), VAO(0), VBO(0), EBO(0), normalVBO(0), elevationVBO(0) {
    generateTerrain();
    calculatePosition();
    setupBuffers();
}

CelestialBody::~CelestialBody() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &normalVBO);
        glDeleteBuffers(1, &elevationVBO);
    }
}

void CelestialBody::generateTerrain() {
    // Configure terrain for moon-like appearance
    TerrainConfig moonConfig;
    moonConfig.continentAmplitude = 0.1f;   // Less dramatic terrain
    moonConfig.continentFrequency = 0.8f;
    moonConfig.mountainAmplitude = 0.15f;   // Crater-like features
    moonConfig.mountainFrequency = 3.0f;
    moonConfig.hillAmplitude = 0.08f;
    moonConfig.hillFrequency = 6.0f;
    moonConfig.detailAmplitude = 0.04f;
    moonConfig.detailFrequency = 12.0f;
    moonConfig.oceanLevel = -1.0f;          // No oceans on moon
    moonConfig.maxElevation = 0.3f;
    
    mesh.generateTerrain(moonConfig);
    
    // Add some crater-like features
    FastNoiseLite craterNoise;
    craterNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    craterNoise.SetFrequency(2.0f);
    craterNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    craterNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    
    auto& vertices = const_cast<std::vector<glm::vec3>&>(mesh.getVertices());
    auto& elevations = const_cast<std::vector<float>&>(mesh.getElevations());
    
    for (size_t i = 0; i < vertices.size(); i++) {
        glm::vec3 pos = vertices[i];
        float craterValue = craterNoise.GetNoise(pos.x * 10.0f, pos.y * 10.0f, pos.z * 10.0f);
        
        // Create crater depressions
        if (craterValue > 0.3f) {
            float craterDepth = (craterValue - 0.3f) * 0.2f;
            elevations[i] -= craterDepth;
            
            // Displace vertex
            vertices[i] = glm::normalize(pos) * (radius + elevations[i]);
        }
    }
    
    // Recalculate normals after crater generation
    mesh.calculateNormals();
}

void CelestialBody::calculatePosition() {
    float x = orbital.distance * cos(orbital.currentAngle);
    float z = orbital.distance * sin(orbital.currentAngle);
    float y = sin(orbital.inclination) * orbital.distance * sin(orbital.currentAngle) * 0.3f;
    
    position = glm::vec3(x, y, z);
}

void CelestialBody::update(float deltaTime) {
    // Update orbital position
    orbital.currentAngle += orbital.speed * deltaTime;
    
    // Keep angle in valid range
    if (orbital.currentAngle > 2.0f * M_PI) {
        orbital.currentAngle -= 2.0f * M_PI;
    }
    
    calculatePosition();
}

void CelestialBody::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &elevationVBO);

    glBindVertexArray(VAO);

    // Vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.getVertices().size() * sizeof(glm::vec3), &mesh.getVertices()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex normals
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.getNormals().size() * sizeof(glm::vec3), &mesh.getNormals()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Elevation data
    glBindBuffer(GL_ARRAY_BUFFER, elevationVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.getElevations().size() * sizeof(float), &mesh.getElevations()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.getIndices().size() * sizeof(unsigned int), &mesh.getIndices()[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CelestialBody::render(unsigned int shaderProgram) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh.getIndices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
} 