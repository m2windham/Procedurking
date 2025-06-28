#pragma once
#include <glm/glm.hpp>
#include "Icosphere.h"

struct OrbitalParams {
    float distance;         // Distance from parent body
    float speed;           // Orbital speed
    float inclination;     // Orbital inclination in radians
    float eccentricity;    // Orbital eccentricity (0 = circular)
    float currentAngle;    // Current position in orbit
};

class CelestialBody {
public:
    CelestialBody(float radius, int subdivisions, const OrbitalParams& orbit);
    ~CelestialBody();
    
    void update(float deltaTime);
    void render(unsigned int shaderProgram);
    void setupBuffers();
    
    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    const Icosphere& getMesh() const { return mesh; }
    
    // Rendering data
    unsigned int getVAO() const { return VAO; }
    
private:
    Icosphere mesh;
    OrbitalParams orbital;
    glm::vec3 position;
    float radius;
    
    // OpenGL buffers
    unsigned int VAO, VBO, EBO, normalVBO, elevationVBO;
    
    void calculatePosition();
    void generateTerrain();
}; 