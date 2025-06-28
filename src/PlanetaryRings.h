#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <random>

struct RingParticle {
    glm::vec3 position;
    float size;
    float alpha;
    float orbitalSpeed;
    float angle;
};

class PlanetaryRings {
public:
    PlanetaryRings(int numParticles = 50000);
    ~PlanetaryRings();
    
    void update(float deltaTime);
    void render();
    void setupBuffers();
    
    const std::vector<RingParticle>& getParticles() const { return particles; }
    unsigned int getVAO() const { return VAO; }
    
private:
    std::vector<RingParticle> particles;
    unsigned int VAO, quadVBO, instanceVBO;
    float innerRadius;
    float outerRadius;
    
    void generateParticles(int numParticles);
    void updateInstanceBuffer();
    
    // Quad vertices for billboarded particles
    static const float quadVertices[];
}; 