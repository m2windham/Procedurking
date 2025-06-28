#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include <vector>
#include <queue>
#include <mutex>

// ============================================================================
// FIRE SIMULATION SYSTEM
// ============================================================================

/**
 * Fire state for individual voxels
 */
struct VoxelFireState {
    bool isOnFire;
    float temperature;        // Current temperature (Celsius)
    float fuelLevel;         // Remaining combustible material (0-1)
    float oxygenLevel;       // Available oxygen (0-1)
    float ignitionTime;      // Time since ignition
    float burnIntensity;     // Current burn rate (0-1)
    
    // Fire spread parameters
    float heatTransferRate;  // How fast heat spreads to neighbors
    glm::vec3 windEffect;    // Wind influence on fire spread
    bool hasSpreadToday;     // Prevents infinite spread in one update
    
    VoxelFireState()
        : isOnFire(false), temperature(20.0f), fuelLevel(1.0f), oxygenLevel(1.0f),
          ignitionTime(0.0f), burnIntensity(0.0f), heatTransferRate(0.1f),
          windEffect(0.0f), hasSpreadToday(false) {}
};

/**
 * Cellular automata fire simulation
 */
class FireSimulation {
public:
    FireSimulation(VoxelWorldManager& worldManager, const MaterialPalette& palette);
    
    // Fire management
    void igniteVoxel(const VoxelPos& pos, float initialTemperature = 800.0f);
    void extinguishVoxel(const VoxelPos& pos);
    void setWindVector(const glm::vec3& wind) { globalWind = wind; }
    
    // Simulation update
    void update(float deltaTime);  // [GPU_CANDIDATE]
    void updateChunk(const ChunkPos& chunkPos, float deltaTime);  // [GPU_CANDIDATE]
    
    // Fire queries
    bool isVoxelOnFire(const VoxelPos& pos) const;
    float getVoxelTemperature(const VoxelPos& pos) const;
    std::vector<VoxelPos> getFireVoxelsInRadius(const glm::vec3& center, float radius) const;
    
    // Configuration
    void setMaxActiveFireSources(uint32_t maxSources) { maxActiveFireSources = maxSources; }
    void setFireSpreadRate(float rate) { fireSpreadRate = rate; }
    void setEnableSmoke(bool enable) { enableSmoke = enable; }
    
    // Statistics
    struct FireStats {
        uint32_t activeFireVoxels;
        uint32_t totalVoxelsBurned;
        float averageTemperature;
        float totalHeatGenerated;
        uint32_t fireSourcesActive;
    };
    FireStats getStatistics() const { return fireStats; }
    
private:
    VoxelWorldManager& worldManager;
    const MaterialPalette& materialPalette;
    
    // Fire state storage
    std::unordered_map<VoxelPos, VoxelFireState, VoxelPosHash> fireStates;
    std::queue<VoxelPos> activeFireVoxels;
    
    // Global parameters
    glm::vec3 globalWind;
    uint32_t maxActiveFireSources;
    float fireSpreadRate;
    bool enableSmoke;
    
    // Statistics
    mutable FireStats fireStats;
    mutable std::mutex fireStatsMutex;
    
    // Core fire algorithms
    void propagateHeat(const VoxelPos& pos, float deltaTime);  // [GPU_CANDIDATE]
    void updateFireState(VoxelFireState& fireState, uint8_t materialID, float deltaTime);
    void checkIgnitionConditions(const VoxelPos& pos, VoxelFireState& fireState);
    void calculateHeatTransfer(const VoxelPos& source, const VoxelPos& target, float deltaTime);
    
    // Material fire properties
    float getMaterialFlashPoint(uint8_t materialID) const;
    float getMaterialBurnRate(uint8_t materialID) const;
    float getMaterialHeatCapacity(uint8_t materialID) const;
    bool isMaterialFlammable(uint8_t materialID) const;
    
    // Wind and environmental effects
    glm::vec3 calculateWindEffect(const VoxelPos& pos) const;
    float getOxygenLevel(const VoxelPos& pos) const;
    void updateTemperatureDistribution();
    
    // Performance optimization
    void limitActiveFireSources();
    void cleanupExtinguishedFires();
    void updateFireStatistics();
};

// ============================================================================
// FLUID SIMULATION SYSTEM
// ============================================================================

/**
 * Fluid cell state for coarse grid simulation
 */
struct FluidCell {
    float pressure;          // Fluid pressure (Pa)
    glm::vec3 velocity;      // Fluid velocity (m/s)
    float density;           // Fluid density (kg/m³)
    float temperature;       // Fluid temperature (Celsius)
    
    // Fluid type and properties
    uint8_t fluidType;       // Water, lava, oil, etc.
    float viscosity;         // Dynamic viscosity
    float volume;            // Fluid volume in cell (0-1)
    
    // State flags
    bool isSolid;            // True if cell contains solid voxels
    bool hasFlow;            // True if fluid is moving
    bool isAtSurface;        // True if exposed to air
    
    FluidCell()
        : pressure(101325.0f), velocity(0.0f), density(1000.0f), temperature(20.0f),
          fluidType(0), viscosity(0.001f), volume(0.0f),
          isSolid(false), hasFlow(false), isAtSurface(false) {}
};

/**
 * Grid-based fluid dynamics simulation
 */
class FluidSimulation {
public:
    FluidSimulation(VoxelWorldManager& worldManager, const MaterialPalette& palette);
    ~FluidSimulation();
    
    // Fluid management
    void addFluid(const glm::vec3& position, float volume, uint8_t fluidType);
    void removeFluid(const glm::vec3& position, float volume);
    void setFluidSource(const glm::vec3& position, float flowRate, uint8_t fluidType);
    
    // Simulation update
    void update(float deltaTime);  // [GPU_CANDIDATE]
    void updateFluidChunk(const ChunkPos& chunkPos, float deltaTime);  // [GPU_CANDIDATE]
    
    // Fluid queries
    float getFluidLevel(const glm::vec3& position) const;
    glm::vec3 getFluidVelocity(const glm::vec3& position) const;
    float getFluidPressure(const glm::vec3& position) const;
    
    // Voxel interaction
    void handleVoxelDestruction(const VoxelPos& pos);
    void handleVoxelCreation(const VoxelPos& pos);
    void updateFluidAfterDestruction(const std::vector<VoxelPos>& destroyedVoxels);
    
    // Configuration
    void setGravity(const glm::vec3& gravity) { gravityVector = gravity; }
    void setFluidGridResolution(float resolution) { gridResolution = resolution; }
    void setEnableEvaporation(bool enable) { enableEvaporation = enable; }
    
    // Statistics
    struct FluidStats {
        uint32_t activeFluidCells;
        float totalFluidVolume;
        float averageFlowRate;
        uint32_t fluidSources;
        float simulationTime;
    };
    FluidStats getStatistics() const { return fluidStats; }
    
private:
    VoxelWorldManager& worldManager;
    const MaterialPalette& materialPalette;
    
    // Fluid grid storage
    struct FluidGrid {
        std::vector<FluidCell> cells;
        int32_t width, height, depth;
        glm::vec3 origin;
        float cellSize;
        
        FluidGrid(int32_t w, int32_t h, int32_t d, const glm::vec3& orig, float size)
            : width(w), height(h), depth(d), origin(orig), cellSize(size) {
            cells.resize(w * h * d);
        }
        
        FluidCell& getCell(int32_t x, int32_t y, int32_t z) {
            return cells[x + y * width + z * width * height];
        }
        
        const FluidCell& getCell(int32_t x, int32_t y, int32_t z) const {
            return cells[x + y * width + z * width * height];
        }
    };
    
    std::unordered_map<ChunkPos, std::unique_ptr<FluidGrid>, ChunkPosHash> fluidGrids;
    
    // Fluid sources and sinks
    struct FluidSource {
        glm::vec3 position;
        float flowRate;         // m³/s
        uint8_t fluidType;
        bool isActive;
    };
    std::vector<FluidSource> fluidSources;
    
    // Global parameters
    glm::vec3 gravityVector;
    float gridResolution;
    bool enableEvaporation;
    float timeStep;
    
    // Statistics
    mutable FluidStats fluidStats;
    mutable std::mutex fluidStatsMutex;
    
    // Core fluid algorithms
    void solvePressure(FluidGrid& grid, float deltaTime);  // [GPU_CANDIDATE]
    void advectVelocity(FluidGrid& grid, float deltaTime);  // [GPU_CANDIDATE]
    void applyGravity(FluidGrid& grid, float deltaTime);
    void enforceIncompressibility(FluidGrid& grid);  // [GPU_CANDIDATE]
    
    // Pressure solver (simplified)
    void jacobiIteration(FluidGrid& grid, int32_t iterations);
    void calculateDivergence(FluidGrid& grid, std::vector<float>& divergence);
    void applyPressureGradient(FluidGrid& grid, const std::vector<float>& pressure);
    
    // Boundary conditions
    void updateBoundaryConditions(FluidGrid& grid);
    void handleSolidBoundaries(FluidGrid& grid);
    void handleFreeSurface(FluidGrid& grid);
    
    // Voxel-fluid interaction
    void updateFluidSolidBoundary(const VoxelPos& voxelPos, FluidGrid& grid);
    void redistributeFluid(const VoxelPos& destroyedVoxel, FluidGrid& grid);
    bool isVoxelSolid(const VoxelPos& pos) const;
    
    // Grid management
    std::unique_ptr<FluidGrid> createFluidGrid(const ChunkPos& chunkPos);
    void destroyFluidGrid(const ChunkPos& chunkPos);
    glm::ivec3 worldToGridCoords(const glm::vec3& worldPos, const FluidGrid& grid) const;
    glm::vec3 gridToWorldCoords(const glm::ivec3& gridPos, const FluidGrid& grid) const;
    
    // Fluid properties
    float getFluidDensity(uint8_t fluidType) const;
    float getFluidViscosity(uint8_t fluidType) const;
    glm::vec3 getFluidColor(uint8_t fluidType) const;
    
    // Performance optimization
    void optimizeFluidGrids();
    void cleanupEmptyGrids();
    void updateFluidStatistics();
};

// ============================================================================
// ENVIRONMENTAL EFFECTS MANAGER
// ============================================================================

/**
 * Manages environmental effects like weather, erosion, and chemical reactions
 */
class VoxelClimateSimulator {
public:
    VoxelClimateSimulator(VoxelWorldManager& worldManager, const MaterialPalette& palette);
    ~VoxelClimateSimulator();
    
    // Primary simulation systems
    FireSimulation& getFireSimulation() { return *fireSimulation; }
    FluidSimulation& getFluidSimulation() { return *fluidSimulation; }
    
    // Integrated effects
    void update(float deltaTime);
    void handleDestructionEvent(const DestructionEvent& event);
    
    // Weather simulation
    void setWeatherConditions(float temperature, float humidity, float windSpeed, const glm::vec3& windDirection);
    void simulateRain(const glm::vec3& center, float radius, float intensity, float duration);
    void simulateSnow(const glm::vec3& center, float radius, float intensity, float duration);
    void simulateStorm(const glm::vec3& center, float radius, float intensity);
    
    // Chemical reactions
    void simulateAcidRain(const glm::vec3& center, float radius, float acidity);
    void simulateCorrosion(const VoxelPos& pos, float rate, float deltaTime);
    void simulateOxidation(const VoxelPos& pos, float rate, float deltaTime);
    
    // Erosion and geological processes
    void simulateWaterErosion(const glm::vec3& center, float radius, float rate);
    void simulateWindErosion(const glm::vec3& center, float radius, const glm::vec3& windDirection);
    void simulateFreezeThawCycles(const glm::vec3& center, float radius);
    
    // Temperature effects
    void simulateHeatTransfer(float deltaTime);  // [GPU_CANDIDATE]
    void simulateFreezing(const VoxelPos& pos, float temperature);
    void simulateMelting(const VoxelPos& pos, float temperature);
    void simulateEvaporation(const VoxelPos& pos, float temperature, float humidity);
    
    // Configuration
    void setGlobalTemperature(float temperature) { globalTemperature = temperature; }
    void setGlobalHumidity(float humidity) { globalHumidity = humidity; }
    void setSeasonalEffects(bool enable) { enableSeasonalEffects = enable; }
    
    // Statistics
    struct ClimateStats {
        FireSimulation::FireStats fireStats;
        FluidSimulation::FluidStats fluidStats;
        float globalTemperature;
        float globalHumidity;
        uint32_t activeWeatherEvents;
        float erosionRate;
    };
    ClimateStats getStatistics() const;
    
private:
    VoxelWorldManager& worldManager;
    const MaterialPalette& materialPalette;
    
    // Simulation systems
    std::unique_ptr<FireSimulation> fireSimulation;
    std::unique_ptr<FluidSimulation> fluidSimulation;
    
    // Weather state
    struct WeatherState {
        float temperature;
        float humidity;
        float windSpeed;
        glm::vec3 windDirection;
        float precipitation;
        bool isStormy;
    } currentWeather;
    
    // Global parameters
    float globalTemperature;
    float globalHumidity;
    bool enableSeasonalEffects;
    float seasonalCycle;  // 0-1 representing year cycle
    
    // Active weather events
    struct WeatherEvent {
        enum Type { RAIN, SNOW, STORM, ACID_RAIN, DROUGHT, HEAT_WAVE } type;
        glm::vec3 center;
        float radius;
        float intensity;
        float duration;
        float timeRemaining;
        
        WeatherEvent(Type t, const glm::vec3& c, float r, float i, float d)
            : type(t), center(c), radius(r), intensity(i), duration(d), timeRemaining(d) {}
    };
    std::vector<WeatherEvent> activeWeatherEvents;
    
    // Update functions
    void updateWeatherEvents(float deltaTime);
    void updateTemperatureDistribution(float deltaTime);
    void updateChemicalReactions(float deltaTime);
    void updateErosionProcesses(float deltaTime);
    
    // Integration between systems
    void handleFireFluidInteraction();
    void handleTemperatureEffects();
    void handleWeatherEffects();
    
    // Helper functions
    float calculateSeasonalTemperature(float baseTemperature) const;
    glm::vec3 calculateWindEffect(const glm::vec3& position) const;
    float getVoxelExposure(const VoxelPos& pos) const;  // Exposure to weather
    
    // Material interaction
    float getMaterialMeltingPoint(uint8_t materialID) const;
    float getMaterialFreezingPoint(uint8_t materialID) const;
    float getMaterialCorrosionRate(uint8_t materialID) const;
    bool isMaterialWeatherResistant(uint8_t materialID) const;
}; 