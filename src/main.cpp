#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <FastNoiseLite.h>
#include "Shader.h"
#include "Icosphere.h"
#include "Camera.h"
#include "TerrainSampler.h"
#include "Starfield.h"
#include "PlanetaryRings.h"
#include "CelestialBody.h"
#include "PlanetManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 4.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

TerrainSampler* terrainSampler = nullptr;
TerrainConfig terrainConfig;
PlanetManager* planetManager = nullptr;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // Enable MSAA

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Procedurking - Planet Explorer Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE); // Enable MSAA

    // Load shaders
    Shader planetShader("shaders/planet.vert", "shaders/planet.frag");
    Shader atmosphereShader("shaders/atmosphere.vert", "shaders/atmosphere.frag");
    Shader cloudShader("shaders/clouds.vert", "shaders/clouds.frag");
    Shader starShader("shaders/stars.vert", "shaders/stars.frag");
    Shader ringShader("shaders/rings.vert", "shaders/rings.frag");
    Shader moonShader("shaders/planet.vert", "shaders/moon.frag");

    // Create planet with ultra-high detail - SCALED UP 2X
    Icosphere planet(2.0f, 8); // Increased subdivisions for higher resolution
    
    // Configure terrain generation for EXTREME GEOLOGICAL DETAIL
    terrainConfig.continentAmplitude = 0.35f;  // Even more dramatic continental shelves
    terrainConfig.continentFrequency = 0.6f;   // More continents
    terrainConfig.mountainAmplitude = 1.0f;    // Even larger mountain ranges
    terrainConfig.mountainFrequency = 2.5f;    // More mountain chains
    terrainConfig.hillAmplitude = 0.5f;        // More pronounced hills
    terrainConfig.hillFrequency = 4.5f;        // Denser hills
    terrainConfig.detailAmplitude = 0.25f;     // More dramatic surface features
    terrainConfig.detailFrequency = 10.0f;     // Even denser geological detail
    terrainConfig.oceanLevel = 0.08f;          // Slightly deeper oceans
    terrainConfig.maxElevation = 1.2f;         // Higher elevation range
    
    planet.generateTerrain(terrainConfig);
    
    // Procedural flora and fauna generation
    GenerateProceduralPlants(&planet, terrainConfig);
    GenerateProceduralAnimals(&planet, terrainConfig);
    
    // Create atmosphere and cloud spheres - SCALED UP 2X
    Icosphere atmosphere(2.1f, 4); // Was 1.05f, now 2.1f (slightly larger than planet)
    Icosphere clouds(2.04f, 5);    // Was 1.02f, now 2.04f (between planet and atmosphere)
    
    // Create starfield background
    Starfield starfield(8000, 500.0f);
    
    // Create planetary ring system - SCALED UP 2X
    PlanetaryRings rings(25000);
    
    // Create moon system - SCALED UP 2X
    std::vector<std::unique_ptr<CelestialBody>> moons;
    
    // Large moon - close orbit (scaled up)
    OrbitalParams moon1Orbit = {12.0f, 0.3f, 0.1f, 0.0f, 0.0f}; // Was 6.0f, now 12.0f
    moons.push_back(std::make_unique<CelestialBody>(0.54f, 5, moon1Orbit)); // Was 0.27f, now 0.54f
    
    // Small moon - distant orbit (scaled up)
    OrbitalParams moon2Orbit = {24.0f, 0.15f, 0.3f, 0.1f, 1.57f}; // Was 12.0f, now 24.0f
    moons.push_back(std::make_unique<CelestialBody>(0.3f, 4, moon2Orbit)); // Was 0.15f, now 0.3f
    
    // Tiny moon - eccentric orbit (scaled up)
    OrbitalParams moon3Orbit = {17.0f, 0.25f, 0.05f, 0.3f, 3.14f}; // Was 8.5f, now 17.0f
    moons.push_back(std::make_unique<CelestialBody>(0.16f, 3, moon3Orbit)); // Was 0.08f, now 0.16f
    
    // Create terrain sampler for collision detection
    terrainSampler = new TerrainSampler(&planet);
    
    // Initialize game state
    planetManager = new PlanetManager();

    // Setup planet rendering
    unsigned int planetVAO, planetVBO, planetEBO, planetNormalVBO, planetElevationVBO;
    glGenVertexArrays(1, &planetVAO);
    glGenBuffers(1, &planetVBO);
    glGenBuffers(1, &planetEBO);
    glGenBuffers(1, &planetNormalVBO);
    glGenBuffers(1, &planetElevationVBO);

    glBindVertexArray(planetVAO);

    // Planet vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, planetVBO);
    glBufferData(GL_ARRAY_BUFFER, planet.getVertices().size() * sizeof(glm::vec3), &planet.getVertices()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Planet vertex normals
    glBindBuffer(GL_ARRAY_BUFFER, planetNormalVBO);
    glBufferData(GL_ARRAY_BUFFER, planet.getNormals().size() * sizeof(glm::vec3), &planet.getNormals()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Planet elevation data
    glBindBuffer(GL_ARRAY_BUFFER, planetElevationVBO);
    glBufferData(GL_ARRAY_BUFFER, planet.getElevations().size() * sizeof(float), &planet.getElevations()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    // Planet indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.getIndices().size() * sizeof(unsigned int), &planet.getIndices()[0], GL_STATIC_DRAW);

    // Setup atmosphere rendering
    unsigned int atmosphereVAO, atmosphereVBO, atmosphereEBO;
    glGenVertexArrays(1, &atmosphereVAO);
    glGenBuffers(1, &atmosphereVBO);
    glGenBuffers(1, &atmosphereEBO);

    glBindVertexArray(atmosphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, atmosphereVBO);
    glBufferData(GL_ARRAY_BUFFER, atmosphere.getVertices().size() * sizeof(glm::vec3), &atmosphere.getVertices()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atmosphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, atmosphere.getIndices().size() * sizeof(unsigned int), &atmosphere.getIndices()[0], GL_STATIC_DRAW);

    // Setup cloud rendering
    unsigned int cloudVAO, cloudVBO, cloudEBO;
    glGenVertexArrays(1, &cloudVAO);
    glGenBuffers(1, &cloudVBO);
    glGenBuffers(1, &cloudEBO);

    glBindVertexArray(cloudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cloudVBO);
    glBufferData(GL_ARRAY_BUFFER, clouds.getVertices().size() * sizeof(glm::vec3), &clouds.getVertices()[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, clouds.getIndices().size() * sizeof(unsigned int), &clouds.getIndices()[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "=== PROCEDURKING - PLANET GOD GAME ===" << std::endl;
    std::cout << "Movement Controls:" << std::endl;
    std::cout << "WASD - Move" << std::endl;
    std::cout << "Mouse - Look around" << std::endl;
    std::cout << "F - Toggle between fly and walk mode (isometric god view)" << std::endl;
    std::cout << "Space - Jump (walk mode only)" << std::endl;
    std::cout << "Q/E - Up/Down (fly mode only)" << std::endl;
    std::cout << "Scroll - Zoom" << std::endl;
    std::cout << std::endl;
    std::cout << "God Game Controls:" << std::endl;
    std::cout << "1-5 - Adjust Solar Energy (watch ice caps/vegetation)" << std::endl;
    std::cout << "6-0 - Adjust Volcanism (see glowing lava)" << std::endl;
    std::cout << "R - Discover Elements (explore different elevations)" << std::endl;
    std::cout << "T - Accelerate Time (watch evolution)" << std::endl;
    std::cout << "L - Trigger Life Emergence (manual start)" << std::endl;
    std::cout << "TAB - Show Planet Status" << std::endl;
    std::cout << std::endl;
    std::cout << "Features: Procedural life evolution with multiple chemistries!" << std::endl;
    std::cout << "TIP: Use F for isometric god view to see planetary changes!" << std::endl;

    // Variables for HUD updates
    float lastHudUpdate = 0.0f;
    float hudUpdateInterval = 1.0f; // Update HUD every 1 second
    
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        // Update HUD periodically
        if (currentFrame - lastHudUpdate > hudUpdateInterval) {
            // Clear console and show current status
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            
            const LifeProgress& life = planetManager->getLifeProgress();
            float timeSpeed = planetManager->getTimeAcceleration();
            
            std::cout << "=== PROCEDURKING - REAL-TIME STATUS ===" << std::endl;
            std::cout << "Time Speed: " << timeSpeed << "x" << std::endl;
            std::cout << "Solar Energy: " << planetManager->getGlobalCondition("solarEnergy") << "x ";
            if (planetManager->getGlobalCondition("solarEnergy") < 0.8f) std::cout << "(ICE AGE)";
            else if (planetManager->getGlobalCondition("solarEnergy") > 1.3f) std::cout << "(HOT HOUSE)";
            else std::cout << "(TEMPERATE)";
            std::cout << std::endl;
            
            std::cout << "Volcanism: " << planetManager->getGlobalCondition("volcanism") << "x ";
            if (planetManager->getGlobalCondition("volcanism") < 0.5f) std::cout << "(DORMANT)";
            else if (planetManager->getGlobalCondition("volcanism") > 1.5f) std::cout << "(ACTIVE ERUPTIONS!)";
            else std::cout << "(MODERATE)";
            std::cout << std::endl;
            
            std::cout << "Life Stage: " << planetManager->getCurrentStageDescription() << " (" << (int)(life.stageProgress * 100) << "%)" << std::endl;
            std::cout << "Habitability: " << (int)(planetManager->getPlanetHabitability() * 100) << "%" << std::endl;
            
            // Display life evolution details
            auto lifeforms = planetManager->getLifeFormDescriptions();
            if (lifeforms.size() > 1) {
                std::cout << "Active Species: " << (lifeforms.size() - 4) << std::endl; // Subtract status lines
            }
            
            if (camera.Mode == WALK && glm::length(camera.Position) > 2.6f) {
                std::cout << "VIEW: Isometric God Mode (5000ft altitude)" << std::endl;
            } else if (camera.Mode == WALK) {
                std::cout << "VIEW: Surface Level" << std::endl;
            } else {
                std::cout << "VIEW: Space Flight" << std::endl;
            }
            
            std::cout << "===========================================" << std::endl;
            
            lastHudUpdate = currentFrame;
        }

        // Update systems
        rings.update(deltaTime);
        for (auto& moon : moons) {
            moon->update(deltaTime);
        }
        
        // Update camera physics and terrain collision with FIXED sampling for 2x scale
        if (camera.Mode == WALK) {
            // Get the full radius (planet radius + elevation) from terrain sampler
            float fullRadius = terrainSampler->GetHeightAtPosition(camera.Position);
            // The terrain height in world space is the full radius
            camera.SetGroundHeight(fullRadius);
        }
        camera.UpdatePhysics(deltaTime);
        
        // Update game state with CORRECTED elevation calculation
        float currentElevation = 0.0f;
        if (terrainSampler) {
            float fullRadius = terrainSampler->GetHeightAtPosition(camera.Position);
            currentElevation = fullRadius - 2.0f; // Subtract planet radius (2.0f for 2x scale)
        }
        planetManager->update(deltaTime);

        // Clear with deep space color
        glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Common uniforms - much closer near plane for tiny player perspective
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.0001f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 lightDirection = glm::normalize(glm::vec3(0.3f, 0.8f, 0.5f));

        // Render starfield first (background)
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_PROGRAM_POINT_SIZE);
        starShader.use();
        starShader.setMat4("view", view);
        starShader.setMat4("projection", projection);
        starShader.setFloat("time", currentFrame);
        starfield.render();
        glDisable(GL_PROGRAM_POINT_SIZE);
        glDepthFunc(GL_LESS);

        // Render planet
        planetShader.use();
        planetShader.setVec3("viewPos", camera.Position);
        planetShader.setFloat("maxElevation", terrainConfig.maxElevation);
        planetShader.setFloat("oceanLevel", terrainConfig.oceanLevel);
        planetShader.setFloat("time", currentFrame);
        planetShader.setFloat("solarEnergy", planetManager->getGlobalCondition("solarEnergy"));
        planetShader.setFloat("volcanism", planetManager->getGlobalCondition("volcanism"));
        planetShader.setFloat("timeSpeed", planetManager->getTimeAcceleration());
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setMat4("model", model);
        
        glBindVertexArray(planetVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)planet.getIndices().size(), GL_UNSIGNED_INT, 0);

        // Render moons
        moonShader.use();
        moonShader.setVec3("viewPos", camera.Position);
        moonShader.setVec3("lightDir", lightDirection);
        moonShader.setFloat("maxElevation", 0.3f);
        moonShader.setMat4("projection", projection);
        moonShader.setMat4("view", view);
        
        for (auto& moon : moons) {
            glm::mat4 moonModel = glm::mat4(1.0f);
            moonModel = glm::translate(moonModel, moon->getPosition());
            moonShader.setMat4("model", moonModel);
            
            glBindVertexArray(moon->getVAO());
            glDrawElements(GL_TRIANGLES, (GLsizei)moon->getMesh().getIndices().size(), GL_UNSIGNED_INT, 0);
        }

        // Render planetary rings with transparency
        glDepthMask(GL_FALSE);
        ringShader.use();
        ringShader.setVec3("viewPos", camera.Position);
        ringShader.setVec3("lightDir", lightDirection);
        ringShader.setFloat("time", currentFrame);
        ringShader.setMat4("projection", projection);
        ringShader.setMat4("view", view);
        ringShader.setMat4("model", model);
        rings.render();

        // Render clouds with transparency
        cloudShader.use();
        cloudShader.setVec3("viewPos", camera.Position);
        cloudShader.setVec3("lightDir", lightDirection);
        cloudShader.setFloat("time", currentFrame);
        cloudShader.setMat4("projection", projection);
        cloudShader.setMat4("view", view);
        cloudShader.setMat4("model", model);
        
        glBindVertexArray(cloudVAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)clouds.getIndices().size(), GL_UNSIGNED_INT, 0);

        // Render atmosphere (only when far from planet)
        float distanceFromCenter = glm::length(camera.Position);
        if (distanceFromCenter > 3.0f) {
            atmosphereShader.use();
            atmosphereShader.setVec3("viewPos", camera.Position);
            atmosphereShader.setVec3("lightDir", lightDirection);
            atmosphereShader.setFloat("atmosphereRadius", 2.1f);
            atmosphereShader.setFloat("planetRadius", 2.0f);
            atmosphereShader.setMat4("projection", projection);
            atmosphereShader.setMat4("view", view);
            atmosphereShader.setMat4("model", model);
            
            glBindVertexArray(atmosphereVAO);
            glDrawElements(GL_TRIANGLES, (GLsizei)atmosphere.getIndices().size(), GL_UNSIGNED_INT, 0);
        }
        
        glDepthMask(GL_TRUE); // Re-enable depth writing

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &planetVAO);
    glDeleteBuffers(1, &planetVBO);
    glDeleteBuffers(1, &planetEBO);
    glDeleteBuffers(1, &planetNormalVBO);
    glDeleteBuffers(1, &planetElevationVBO);
    
    glDeleteVertexArrays(1, &atmosphereVAO);
    glDeleteBuffers(1, &atmosphereVBO);
    glDeleteBuffers(1, &atmosphereEBO);
    
    glDeleteVertexArrays(1, &cloudVAO);
    glDeleteBuffers(1, &cloudVBO);
    glDeleteBuffers(1, &cloudEBO);
    
    delete terrainSampler;
    delete planetManager;

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    static bool fKeyPressed = false;
    static bool rKeyPressed = false;
    static bool tKeyPressed = false;
    static bool tabKeyPressed = false;
    static bool key1Pressed = false;
    static bool key2Pressed = false;
    static bool key3Pressed = false;
    static bool key4Pressed = false;
    static bool key5Pressed = false;
    static bool key6Pressed = false;
    static bool key7Pressed = false;
    static bool key8Pressed = false;
    static bool key9Pressed = false;
    static bool key0Pressed = false;
    static bool lKeyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(JUMP, deltaTime);

    // Toggle camera mode
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyPressed) {
        camera.ToggleMode();
        fKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyPressed = false;
    }
    
    // Solar Energy Control (1-5 keys)
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        planetManager->setGlobalCondition("solarEnergy", 0.5f);
        std::cout << "Solar Energy set to LOW (0.5x)" << std::endl;
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) { key1Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        planetManager->setGlobalCondition("solarEnergy", 0.8f);
        std::cout << "Solar Energy set to REDUCED (0.8x)" << std::endl;
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) { key2Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
        planetManager->setGlobalCondition("solarEnergy", 1.0f);
        std::cout << "Solar Energy set to EARTH-LIKE (1.0x)" << std::endl;
        key3Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) { key3Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed) {
        planetManager->setGlobalCondition("solarEnergy", 1.3f);
        std::cout << "Solar Energy set to HIGH (1.3x)" << std::endl;
        key4Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) { key4Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !key5Pressed) {
        planetManager->setGlobalCondition("solarEnergy", 1.8f);
        std::cout << "Solar Energy set to EXTREME (1.8x)" << std::endl;
        key5Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) { key5Pressed = false; }
    
    // Volcanism Control (6-0 keys)
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !key6Pressed) {
        planetManager->setGlobalCondition("volcanism", 0.1f);
        std::cout << "Volcanism set to DORMANT (0.1x)" << std::endl;
        key6Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE) { key6Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !key7Pressed) {
        planetManager->setGlobalCondition("volcanism", 0.5f);
        std::cout << "Volcanism set to LOW (0.5x)" << std::endl;
        key7Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE) { key7Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS && !key8Pressed) {
        planetManager->setGlobalCondition("volcanism", 1.0f);
        std::cout << "Volcanism set to MODERATE (1.0x)" << std::endl;
        key8Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE) { key8Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS && !key9Pressed) {
        planetManager->setGlobalCondition("volcanism", 1.5f);
        std::cout << "Volcanism set to HIGH (1.5x)" << std::endl;
        key9Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE) { key9Pressed = false; }
    
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !key0Pressed) {
        planetManager->setGlobalCondition("volcanism", 2.0f);
        std::cout << "Volcanism set to EXTREME (2.0x)" << std::endl;
        key0Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE) { key0Pressed = false; }
    
    // Element Discovery
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !rKeyPressed) {
        float currentElevation = 0.0f;
        if (terrainSampler) {
            float fullRadius = terrainSampler->GetHeightAtPosition(camera.Position);
            currentElevation = fullRadius - 2.0f; // Subtract planet radius
        }
        
        bool discovered = planetManager->discoverElement(camera.Position, currentElevation);
        if (!discovered) {
            std::cout << "No elements discovered here. Try exploring different elevations!" << std::endl;
        }
        
        rKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        rKeyPressed = false;
    }
    
    // Time Acceleration
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tKeyPressed) {
        static float timeSpeed = 1.0f;
        timeSpeed *= 2.0f;
        if (timeSpeed > 8.0f) timeSpeed = 0.5f;
        planetManager->accelerateEvolution(timeSpeed);
        tKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        tKeyPressed = false;
    }
    
    // Show planet status
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabKeyPressed) {
        const LifeProgress& life = planetManager->getLifeProgress();
        
        std::cout << "\n=== PLANET GOD STATUS ===" << std::endl;
        std::cout << "Life Stage: " << planetManager->getCurrentStageDescription() << std::endl;
        std::cout << "Progress: " << (int)(life.stageProgress * 100) << "%" << std::endl;
        std::cout << "Habitability: " << (int)(planetManager->getPlanetHabitability() * 100) << "%" << std::endl;
        
        std::cout << "\nPlanetary Conditions:" << std::endl;
        std::cout << "Solar Energy: " << planetManager->getGlobalCondition("solarEnergy") << "x" << std::endl;
        std::cout << "Volcanism: " << planetManager->getGlobalCondition("volcanism") << "x" << std::endl;
        std::cout << "Gravity: " << planetManager->getGlobalCondition("gravity") << "x" << std::endl;
        
        std::cout << "\nElements:" << std::endl;
        std::cout << "Water: " << (int)(planetManager->getElementAbundance(ElementType::WATER) * 100) << "%" << std::endl;
        std::cout << "Carbon: " << (int)(planetManager->getElementAbundance(ElementType::CARBON) * 100) << "%" << std::endl;
        std::cout << "Oxygen: " << (int)(planetManager->getElementAbundance(ElementType::OXYGEN) * 100) << "%" << std::endl;
        std::cout << "Iron: " << (int)(planetManager->getElementAbundance(ElementType::IRON) * 100) << "%" << std::endl;
        std::cout << "Rare Earth: " << (int)(planetManager->getElementAbundance(ElementType::RARE_EARTH) * 100) << "%" << std::endl;
        
        if (life.currentStage > LifeStage::STERILE) {
            std::cout << "\nLife Statistics:" << std::endl;
            std::cout << "Biomass: " << (int)(life.totalBiomass * 100) << "%" << std::endl;
            std::cout << "Diversity: " << (int)life.diversity << " species" << std::endl;
            if (life.intelligence > 0) {
                std::cout << "Intelligence: " << (int)(life.intelligence * 100) << "%" << std::endl;
            }
            if (life.technology > 0) {
                std::cout << "Technology: " << (int)(life.technology * 100) << "%" << std::endl;
            }
            
            // Show detailed life forms
            auto lifeforms = planetManager->getLifeFormDescriptions();
            if (lifeforms.size() > 1) {
                std::cout << "\nLife Forms:" << std::endl;
                for (const auto& desc : lifeforms) {
                    if (!desc.empty()) {
                        std::cout << "- " << desc << std::endl;
                    }
                }
            }
        }
        
        auto discoveries = planetManager->getRecentDiscoveries();
        if (!discoveries.empty()) {
            std::cout << "\nRecent Discoveries:" << std::endl;
            for (const auto& discovery : discoveries) {
                std::cout << "- " << discovery << std::endl;
            }
        }
        
        std::cout << "=====================\n" << std::endl;
        
        tabKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabKeyPressed = false;
    }
    
    // Trigger Life Emergence
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lKeyPressed) {
        planetManager->triggerLifeEmergence();
        std::cout << "Life emergence triggered manually!" << std::endl;
        lKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        lKeyPressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window; // Suppress unused parameter warning
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    (void)window; // Suppress unused parameter warning
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window; // Suppress unused parameter warning
    (void)xoffset; // Suppress unused parameter warning
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
} 