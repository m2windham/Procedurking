#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <random>

struct Star {
    glm::vec3 position;
    float brightness;
};

class Starfield {
public:
    Starfield(int numStars = 5000, float radius = 100.0f);
    ~Starfield();
    
    void render();
    void setupBuffers();
    
    const std::vector<Star>& getStars() const { return stars; }
    unsigned int getVAO() const { return VAO; }
    
private:
    std::vector<Star> stars;
    unsigned int VAO, VBO, brightnessVBO;
    float sphereRadius;
    
    void generateStars(int numStars);
    glm::vec3 randomPointOnSphere();
}; 