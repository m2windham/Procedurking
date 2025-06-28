#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include "VoxelPlanetGenerator.h"
#include "VoxelMesher.h"
#include "DestructionManager.h"
#include "StructuralIntegrityManager.h"
#include "VoxelClimateSimulator.h"
#include "VoxelAIDirector.h"
#include "Camera.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>
#include <functional>

// ============================================================================
// RENDERING SYSTEM INTEGRATION
// ============================================================================

/**
 * Voxel-specific rendering configuration
 */
struct VoxelRenderConfig {
    // Rendering quality
    bool enableLOD;              // Level of detail rendering
    bool enableAmbientOcclusion; // Ambient occlusion calculation
    bool enableShadows;          // Real-time shadows
    bool enableReflections;      // Screen-space reflections
    
    // Performance settings
    float renderDistance;        // Maximum render distance
    int32_t maxChunksPerFrame;   // Chunk rendering budget
    bool enableFrustumCulling;   // Frustum culling optimization
    bool enableOcclusionCulling; // Occlusion culling optimization
    
    // Visual effects
    bool enableParticles;        // Particle effects for destruction
    bool enableVolumetricLighting; // Volumetric lighting
    bool enableAtmosphericScattering; // Atmospheric effects
    bool enablePostProcessing;   // Post-processing effects
    
    // Debug visualization
    bool showChunkBoundaries;    // Debug chunk visualization
    bool showStressVisualization; // Structural stress visualization
    bool showFluidFlow;          // Fluid flow visualization
    bool showFireSpread;         // Fire propagation visualization
    
    VoxelRenderConfig()
        : enableLOD(true), enableAmbientOcclusion(true), enableShadows(true), enableReflections(false),
          renderDistance(1000.0f), maxChunksPerFrame(50), enableFrustumCulling(true), enableOcclusionCulling(false),
          enableParticles(true), enableVolumetricLighting(false), enableAtmosphericScattering(true), enablePostProcessing(true),
          showChunkBoundaries(false), showStressVisualization(false), showFluidFlow(false), showFireSpread(false) {}
};

/**
 * Integrated voxel renderer with material-based shading
 */
class VoxelRenderer {
public:
    VoxelRenderer(const MaterialPalette& palette);
    ~VoxelRenderer();
    
    // Initialization
    bool initialize();
    void cleanup();
    
    // Rendering pipeline
    void beginFrame(const Camera& camera, const glm::mat4& projection);
    void renderChunks(const std::vector<std::shared_ptr<ChunkMesh>>& chunks);
    void renderDebris(const std::vector<std::shared_ptr<DebrisObject>>& debris);
    void renderParticleEffects(const std::vector<glm::vec3>& explosions, const std::vector<glm::vec3>& fires);
    void renderDebugVisualization(const VoxelRenderConfig& config);
    void endFrame();
    
    // Configuration
    void setRenderConfig(const VoxelRenderConfig& config) { renderConfig = config; }
    VoxelRenderConfig getRenderConfig() const { return renderConfig; }
    
    // Lighting
    void setDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity);
    void addPointLight(const glm::vec3& position, const glm::vec3& color, float radius);
    void clearLights();
    
    // Performance monitoring
    struct RenderStats {
        uint32_t chunksRendered;
        uint32_t trianglesRendered;
        uint32_t drawCalls;
        float frameTime;
        float gpuMemoryUsed;
    };
    RenderStats getStatistics() const { return renderStats; }
    
private:
    const MaterialPalette& materialPalette;
    VoxelRenderConfig renderConfig;
    
    // Shader programs
    std::unique_ptr<Shader> voxelShader;
    std::unique_ptr<Shader> debrisShader;
    std::unique_ptr<Shader> particleShader;
    std::unique_ptr<Shader> debugShader;
    
    // Rendering resources
    GLuint materialSSBO;         // Material properties buffer
    GLuint lightingSSBO;         // Lighting data buffer
    GLuint cameraUBO;           // Camera uniform buffer
    
    // Framebuffers
    GLuint gBuffer;             // G-buffer for deferred rendering
    GLuint colorTexture;
    GLuint normalTexture;
    GLuint materialTexture;
    GLuint depthTexture;
    
    // Lighting data
    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    } directionalLight;
    
    struct PointLight {
        glm::vec3 position;
        glm::vec3 color;
        float radius;
    };
    std::vector<PointLight> pointLights;
    
    // Statistics
    mutable RenderStats renderStats;
    
    // Initialization helpers
    bool createShaders();
    bool createBuffers();
    bool createFramebuffers();
    
    // Rendering helpers
    void setupLighting();
    void renderChunkMesh(const ChunkMesh& mesh);
    void renderDebrisObject(const DebrisObject& debris);
    void updateMaterialBuffer();
    
    // Debug rendering
    void renderChunkBoundaries(const std::vector<std::shared_ptr<ChunkMesh>>& chunks);
    void renderStressVisualization();
    void renderFluidVisualization();
    void renderFireVisualization();
};

// ============================================================================
// INPUT AND INTERACTION SYSTEM
// ============================================================================

/**
 * Input configuration for voxel interaction
 */
struct VoxelInputConfig {
    // Mouse sensitivity
    float mouseSensitivity;
    float scrollSensitivity;
    
    // Interaction settings
    float interactionRange;      // Maximum interaction distance
    bool enableContinuousDestruction; // Hold to destroy
    bool enablePrecisionMode;    // Fine-grained voxel editing
    
    // Tool settings
    float destructionRadius;     // Default destruction radius
    float constructionRadius;    // Default construction radius
    uint8_t selectedMaterial;    // Currently selected material
    
    VoxelInputConfig()
        : mouseSensitivity(0.1f), scrollSensitivity(1.0f),
          interactionRange(100.0f), enableContinuousDestruction(true), enablePrecisionMode(false),
          destructionRadius(5.0f), constructionRadius(3.0f), selectedMaterial(MaterialPalette::STONE) {}
};

/**
 * Handles player input for voxel world interaction
 */
class VoxelInputHandler {
public:
    VoxelInputHandler(VoxelWorldManager& worldManager, DestructionManager& destructionMgr,
                      VoxelAIDirector& aiDirector, Camera& camera);
    
    // Input processing
    void processInput(GLFWwindow* window, float deltaTime);
    void handleMouseButton(int button, int action, int mods);
    void handleKeyboard(int key, int scancode, int action, int mods);
    void handleMouseMove(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
    
    // Interaction modes
    enum class InteractionMode {
        DESTRUCTION,
        CONSTRUCTION,
        INSPECTION,
        CAMERA_CONTROL
    };
    
    void setInteractionMode(InteractionMode mode) { currentMode = mode; }
    InteractionMode getInteractionMode() const { return currentMode; }
    
    // Configuration
    void setInputConfig(const VoxelInputConfig& config) { inputConfig = config; }
    VoxelInputConfig getInputConfig() const { return inputConfig; }
    
private:
    VoxelWorldManager& worldManager;
    DestructionManager& destructionManager;
    VoxelAIDirector& aiDirector;
    Camera& camera;
    
    VoxelInputConfig inputConfig;
    InteractionMode currentMode;
    
    // Input state
    bool mousePressed[3];
    glm::vec2 lastMousePos;
    bool firstMouse;
    
    // Interaction helpers
    void handleDestruction(const glm::vec3& worldPos, float deltaTime);
    void handleConstruction(const glm::vec3& worldPos, float deltaTime);
    void handleInspection(const glm::vec3& worldPos);
    
    // Raycasting
    glm::vec3 getWorldPositionFromMouse(GLFWwindow* window);
    bool raycastVoxel(const glm::vec3& origin, const glm::vec3& direction, 
                     glm::vec3& hitPoint, VoxelPos& hitVoxel);
};

// ============================================================================
// MAIN SHATTERING ENGINE
// ============================================================================

/**
 * Main engine class that integrates all voxel systems
 * Implements the complete Project Genesis: The Shattering architecture
 */
class VoxelShatteringEngine {
public:
    VoxelShatteringEngine();
    ~VoxelShatteringEngine();
    
    // Engine lifecycle
    bool initialize(int windowWidth, int windowHeight, const std::string& title);
    void run();
    void shutdown();
    
    // World management
    void createNewWorld(uint32_t seed = 42);
    void loadWorld(const std::string& filename);
    void saveWorld(const std::string& filename);
    
    // Player interaction
    void setPlayerPosition(const glm::vec3& position);
    glm::vec3 getPlayerPosition() const;
    void triggerExplosion(const glm::vec3& position, float radius, float damage);
    void addDebris(const glm::vec3& position, const std::vector<VoxelPos>& voxels);
    
    // System access (for advanced users)
    VoxelWorldManager& getWorldManager() { return *worldManager; }
    VoxelPlanetGenerator& getPlanetGenerator() { return *planetGenerator; }
    DestructionManager& getDestructionManager() { return *destructionManager; }
    StructuralIntegrityManager& getStructuralManager() { return *structuralManager; }
    VoxelClimateSimulator& getClimateSimulator() { return *climateSimulator; }
    VoxelAIDirector& getAIDirector() { return *aiDirector; }
    
    // Configuration
    void setRenderConfig(const VoxelRenderConfig& config);
    void setInputConfig(const VoxelInputConfig& config);
    VoxelRenderConfig getRenderConfig() const;
    VoxelInputConfig getInputConfig() const;
    
    // Performance monitoring
    struct EngineStats {
        float frameRate;
        float frameTime;
        VoxelWorldManager::Statistics worldStats;
        DestructionManager::DestructionStats destructionStats;
        StructuralIntegrityManager::StructuralStats structuralStats;
        VoxelClimateSimulator::ClimateStats climateStats;
        VoxelAIDirector::DirectorStats directorStats;
        VoxelRenderer::RenderStats renderStats;
        size_t totalMemoryUsage;
    };
    EngineStats getStatistics() const;
    
    // Event callbacks
    void setOnWorldGenerated(std::function<void()> callback) { onWorldGenerated = callback; }
    void setOnExplosion(std::function<void(const glm::vec3&, float)> callback) { onExplosion = callback; }
    void setOnStructuralCollapse(std::function<void(const glm::vec3&)> callback) { onStructuralCollapse = callback; }
    
private:
    // Core systems (in dependency order)
    std::unique_ptr<MaterialPalette> materialPalette;
    std::unique_ptr<VoxelWorldManager> worldManager;
    std::unique_ptr<VoxelPlanetGenerator> planetGenerator;
    std::unique_ptr<VoxelMeshManager> meshManager;
    std::unique_ptr<DestructionManager> destructionManager;
    std::unique_ptr<StructuralIntegrityManager> structuralManager;
    std::unique_ptr<VoxelClimateSimulator> climateSimulator;
    std::unique_ptr<VoxelAIDirector> aiDirector;
    
    // Rendering and interaction
    std::unique_ptr<VoxelRenderer> renderer;
    std::unique_ptr<VoxelInputHandler> inputHandler;
    std::unique_ptr<Camera> camera;
    
    // Window and OpenGL context
    GLFWwindow* window;
    int windowWidth, windowHeight;
    
    // Engine state
    bool isInitialized;
    bool isRunning;
    VoxelPlanetData planetData;
    
    // Timing
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    float deltaTime;
    float frameRate;
    
    // Event callbacks
    std::function<void()> onWorldGenerated;
    std::function<void(const glm::vec3&, float)> onExplosion;
    std::function<void(const glm::vec3&)> onStructuralCollapse;
    
    // Engine loop
    void updateSystems(float deltaTime);
    void renderFrame();
    void processEvents();
    
    // Initialization phases
    bool initializeOpenGL();
    bool initializeSystems();
    bool generateInitialWorld();
    
    // System integration
    void connectSystemCallbacks();
    void updatePlayerPosition();
    void handleSystemEvents();
    
    // Performance optimization
    void optimizePerformance();
    void balanceSystemLoads();
    void manageMemoryUsage();
    
    // Debug and profiling
    void updateFrameRate();
    void logPerformanceMetrics();
    void validateSystemStates();
    
    // GLFW callbacks (static)
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

// ============================================================================
// CONVENIENCE FUNCTIONS FOR INTEGRATION
// ============================================================================

/**
 * Factory function to create a fully configured Shattering Engine
 */
std::unique_ptr<VoxelShatteringEngine> createShatteringEngine(
    int windowWidth = 1920, 
    int windowHeight = 1080,
    const std::string& title = "Project Genesis: The Shattering"
);

/**
 * Quick setup function for common scenarios
 */
void setupDestructionPlayground(VoxelShatteringEngine& engine, const glm::vec3& playerStart);
void setupConstructionSandbox(VoxelShatteringEngine& engine, const glm::vec3& playerStart);
void setupSurvivalChallenge(VoxelShatteringEngine& engine, const glm::vec3& playerStart);
void setupCreativeMode(VoxelShatteringEngine& engine, const glm::vec3& playerStart); 