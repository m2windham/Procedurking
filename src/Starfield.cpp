#include "Starfield.h"
#include <glad/glad.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static std::random_device rd;
static std::mt19937 gen(rd());

Starfield::Starfield(int numStars, float radius) 
    : sphereRadius(radius), VAO(0), VBO(0), brightnessVBO(0) {
    generateStars(numStars);
    setupBuffers();
}

Starfield::~Starfield() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &brightnessVBO);
    }
}

void Starfield::generateStars(int numStars) {
    stars.clear();
    stars.reserve(numStars);
    
    std::uniform_real_distribution<float> brightnessDist(0.1f, 1.0f);
    
    for (int i = 0; i < numStars; i++) {
        Star star;
        star.position = randomPointOnSphere() * sphereRadius;
        
        // Brightness distribution - more dim stars than bright ones
        float brightness = brightnessDist(gen);
        brightness = brightness * brightness; // Square for more realistic distribution
        star.brightness = brightness;
        
        stars.push_back(star);
    }
}

glm::vec3 Starfield::randomPointOnSphere() {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    
    // Use spherical coordinates for uniform distribution
    float theta = angleDist(gen);  // Azimuthal angle
    float phi = acos(dist(gen));   // Polar angle
    
    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);
    
    return glm::vec3(x, y, z);
}

void Starfield::setupBuffers() {
    // Extract positions and brightness values
    std::vector<glm::vec3> positions;
    std::vector<float> brightness;
    
    positions.reserve(stars.size());
    brightness.reserve(stars.size());
    
    for (const auto& star : stars) {
        positions.push_back(star.position);
        brightness.push_back(star.brightness);
    }
    
    // Generate and bind VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &brightnessVBO);
    
    glBindVertexArray(VAO);
    
    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Brightness buffer
    glBindBuffer(GL_ARRAY_BUFFER, brightnessVBO);
    glBufferData(GL_ARRAY_BUFFER, brightness.size() * sizeof(float), brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Starfield::render() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, (GLsizei)stars.size());
    glBindVertexArray(0);
} 