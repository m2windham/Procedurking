#include "PlanetaryRings.h"
#include <glad/glad.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::random_device rd;
static std::mt19937 gen(rd());

// Quad vertices for billboarded particles (position + texCoord)
const float PlanetaryRings::quadVertices[] = {
    // positions        // texture coords
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
};

PlanetaryRings::PlanetaryRings(int numParticles) 
    : VAO(0), quadVBO(0), instanceVBO(0), innerRadius(3.6f), outerRadius(7.0f) {
    generateParticles(numParticles);
    setupBuffers();
}

PlanetaryRings::~PlanetaryRings() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteBuffers(1, &instanceVBO);
    }
}

void PlanetaryRings::generateParticles(int numParticles) {
    particles.clear();
    particles.reserve(numParticles);
    
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> sizeDist(0.001f, 0.008f);
    std::uniform_real_distribution<float> alphaDist(0.3f, 0.9f);
    std::uniform_real_distribution<float> heightDist(-0.02f, 0.02f);
    
    for (int i = 0; i < numParticles; i++) {
        RingParticle particle;
        
        // Radial distribution with gaps and density variations
        float u = static_cast<float>(i) / numParticles;
        float radius;
        
        // Create realistic ring structure with gaps (scaled up 2x)
        if (u < 0.15f) {
            // Inner sparse region
            radius = innerRadius + (4.4f - innerRadius) * (u / 0.15f);
            if (gen() % 3 != 0) continue; // Skip 2/3 of particles for sparseness
        } else if (u < 0.4f) {
            // Dense main ring
            radius = 4.4f + (5.6f - 4.4f) * ((u - 0.15f) / 0.25f);
        } else if (u < 0.5f) {
            // Cassini gap
            radius = 5.6f + (6.0f - 5.6f) * ((u - 0.4f) / 0.1f);
            if (gen() % 5 != 0) continue; // Very sparse
        } else {
            // Outer ring
            radius = 6.0f + (outerRadius - 6.0f) * ((u - 0.5f) / 0.5f);
        }
        
        particle.angle = angleDist(gen);
        
        // Position with slight vertical variation
        particle.position = glm::vec3(
            radius * cos(particle.angle),
            heightDist(gen),
            radius * sin(particle.angle)
        );
        
        particle.size = sizeDist(gen);
        particle.alpha = alphaDist(gen);
        
        // Orbital speed based on Kepler's laws (closer = faster)
        particle.orbitalSpeed = 0.5f / sqrt(radius);
        
        particles.push_back(particle);
    }
}

void PlanetaryRings::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &instanceVBO);
    
    glBindVertexArray(VAO);
    
    // Setup quad vertices
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Setup instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    
    // Instance position (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    // Instance size (location 3)
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    // Instance alpha (location 4)
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    
    updateInstanceBuffer();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PlanetaryRings::updateInstanceBuffer() {
    std::vector<float> instanceData;
    instanceData.reserve(particles.size() * 5); // pos(3) + size(1) + alpha(1)
    
    for (const auto& particle : particles) {
        instanceData.push_back(particle.position.x);
        instanceData.push_back(particle.position.y);
        instanceData.push_back(particle.position.z);
        instanceData.push_back(particle.size);
        instanceData.push_back(particle.alpha);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float), instanceData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PlanetaryRings::update(float deltaTime) {
    bool needsUpdate = false;
    
    for (auto& particle : particles) {
        // Update orbital position
        particle.angle += particle.orbitalSpeed * deltaTime;
        
        // Keep angle in valid range
        if (particle.angle > 2.0f * M_PI) {
            particle.angle -= 2.0f * M_PI;
        }
        
        // Calculate new position
        float radius = sqrt(particle.position.x * particle.position.x + 
                           particle.position.z * particle.position.z);
        
        particle.position.x = radius * cos(particle.angle);
        particle.position.z = radius * sin(particle.angle);
        
        needsUpdate = true;
    }
    
    if (needsUpdate) {
        updateInstanceBuffer();
    }
}

void PlanetaryRings::render() {
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)particles.size());
    glBindVertexArray(0);
} 