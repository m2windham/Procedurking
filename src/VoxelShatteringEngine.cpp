#include "VoxelShatteringEngine.h"
#include <iostream>

VoxelShatteringEngine::VoxelShatteringEngine()
    : isInitialized(false), isRunning(false), window(nullptr), windowWidth(1920), windowHeight(1080) {
    materialPalette = std::make_unique<MaterialPalette>();
    worldManager = std::make_unique<VoxelWorldManager>(planetData);
    planetGenerator = std::make_unique<VoxelPlanetGenerator>(42);
    meshManager = std::make_unique<VoxelMeshManager>(*materialPalette);
    destructionManager = std::make_unique<DestructionManager>(*worldManager, *materialPalette);
    structuralManager = std::make_unique<StructuralIntegrityManager>(*worldManager, *materialPalette);
    climateSimulator = std::make_unique<VoxelClimateSimulator>(*worldManager, *materialPalette);
    aiDirector = std::make_unique<VoxelAIDirector>(*worldManager, *planetGenerator, *destructionManager, *climateSimulator);
    // renderer, inputHandler, camera not implemented
}

VoxelShatteringEngine::~VoxelShatteringEngine() {}

bool VoxelShatteringEngine::initialize(int windowWidth, int windowHeight, const std::string& title) {
    isInitialized = true;
    return true;
}

void VoxelShatteringEngine::run() {
    isRunning = true;
    while (isRunning) {
        // Minimal: update all systems
        climateSimulator->update(0.016f);
        aiDirector->update(0.016f);
        structuralManager->update(0.016f);
        destructionManager->update(0.016f);
        // ... rendering, input, etc.
        isRunning = false; // Run once for now
    }
}

void VoxelShatteringEngine::shutdown() {
    isRunning = false;
}

void VoxelShatteringEngine::createNewWorld(uint32_t seed) {
    planetData = VoxelPlanetData();
    planetGenerator->setSeed(seed);
    worldManager = std::make_unique<VoxelWorldManager>(planetData);
}

void VoxelShatteringEngine::loadWorld(const std::string& filename) {
    worldManager->loadWorld(filename);
}

void VoxelShatteringEngine::saveWorld(const std::string& filename) {
    worldManager->saveWorld(filename);
}

void VoxelShatteringEngine::setPlayerPosition(const glm::vec3& position) {}
glm::vec3 VoxelShatteringEngine::getPlayerPosition() const { return glm::vec3(0.0f); }
void VoxelShatteringEngine::triggerExplosion(const glm::vec3& position, float radius, float damage) {
    destructionManager->applyExplosion(position, radius, damage);
}
void VoxelShatteringEngine::addDebris(const glm::vec3& position, const std::vector<VoxelPos>& voxels) {}
VoxelWorldManager& VoxelShatteringEngine::getWorldManager() { return *worldManager; }
VoxelPlanetGenerator& VoxelShatteringEngine::getPlanetGenerator() { return *planetGenerator; }
DestructionManager& VoxelShatteringEngine::getDestructionManager() { return *destructionManager; }
StructuralIntegrityManager& VoxelShatteringEngine::getStructuralManager() { return *structuralManager; }
VoxelClimateSimulator& VoxelShatteringEngine::getClimateSimulator() { return *climateSimulator; }
VoxelAIDirector& VoxelShatteringEngine::getAIDirector() { return *aiDirector; }
void VoxelShatteringEngine::setRenderConfig(const VoxelRenderConfig& config) {}
void VoxelShatteringEngine::setInputConfig(const VoxelInputConfig& config) {}
VoxelRenderConfig VoxelShatteringEngine::getRenderConfig() const { return {}; }
VoxelInputConfig VoxelShatteringEngine::getInputConfig() const { return {}; }
VoxelShatteringEngine::EngineStats VoxelShatteringEngine::getStatistics() const { return EngineStats{}; }
void VoxelShatteringEngine::setOnWorldGenerated(std::function<void()> callback) { onWorldGenerated = callback; }
void VoxelShatteringEngine::setOnExplosion(std::function<void(const glm::vec3&, float)> callback) { onExplosion = callback; }
void VoxelShatteringEngine::setOnStructuralCollapse(std::function<void(const glm::vec3&)> callback) { onStructuralCollapse = callback; } 