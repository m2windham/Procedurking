#include "VoxelClimateSimulator.h"
#include <memory>

VoxelClimateSimulator::VoxelClimateSimulator(VoxelWorldManager& worldManager, const MaterialPalette& palette)
    : worldManager(worldManager), materialPalette(palette), globalTemperature(20.0f), globalHumidity(0.5f), enableSeasonalEffects(false), seasonalCycle(0.0f) {
    fireSimulation = std::make_unique<FireSimulation>(worldManager, palette);
    fluidSimulation = std::make_unique<FluidSimulation>(worldManager, palette);
}

VoxelClimateSimulator::~VoxelClimateSimulator() {}

FireSimulation& VoxelClimateSimulator::getFireSimulation() { return *fireSimulation; }
FluidSimulation& VoxelClimateSimulator::getFluidSimulation() { return *fluidSimulation; }

void VoxelClimateSimulator::update(float deltaTime) {
    fireSimulation->update(deltaTime);
    fluidSimulation->update(deltaTime);
}

void VoxelClimateSimulator::handleDestructionEvent(const DestructionEvent& event) {
    // Minimal: no-op
}

void VoxelClimateSimulator::setWeatherConditions(float temperature, float humidity, float windSpeed, const glm::vec3& windDirection) {
    globalTemperature = temperature;
    globalHumidity = humidity;
}

void VoxelClimateSimulator::simulateRain(const glm::vec3& center, float radius, float intensity, float duration) {}
void VoxelClimateSimulator::simulateSnow(const glm::vec3& center, float radius, float intensity, float duration) {}
void VoxelClimateSimulator::simulateStorm(const glm::vec3& center, float radius, float intensity) {}
void VoxelClimateSimulator::simulateAcidRain(const glm::vec3& center, float radius, float acidity) {}
void VoxelClimateSimulator::simulateCorrosion(const VoxelPos& pos, float rate, float deltaTime) {}
void VoxelClimateSimulator::simulateOxidation(const VoxelPos& pos, float rate, float deltaTime) {}
void VoxelClimateSimulator::simulateWaterErosion(const glm::vec3& center, float radius, float rate) {}
void VoxelClimateSimulator::simulateWindErosion(const glm::vec3& center, float radius, const glm::vec3& windDirection) {}
void VoxelClimateSimulator::simulateFreezeThawCycles(const glm::vec3& center, float radius) {}
void VoxelClimateSimulator::simulateHeatTransfer(float deltaTime) {}
void VoxelClimateSimulator::simulateFreezing(const VoxelPos& pos, float temperature) {}
void VoxelClimateSimulator::simulateMelting(const VoxelPos& pos, float temperature) {}
void VoxelClimateSimulator::simulateEvaporation(const VoxelPos& pos, float temperature, float humidity) {}
void VoxelClimateSimulator::setGlobalTemperature(float temperature) { globalTemperature = temperature; }
void VoxelClimateSimulator::setGlobalHumidity(float humidity) { globalHumidity = humidity; }
void VoxelClimateSimulator::setSeasonalEffects(bool enable) { enableSeasonalEffects = enable; }
VoxelClimateSimulator::ClimateStats VoxelClimateSimulator::getStatistics() const { return ClimateStats{}; } 