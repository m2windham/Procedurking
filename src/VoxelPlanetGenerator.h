#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include <FastNoiseLite.h>
#include <random>
#include <thread>
#include <mutex>

// ============================================================================
// NOISE GENERATION SYSTEM
// ============================================================================

/**
 * 3D noise configuration for terrain generation
 */
struct NoiseLayer {
    FastNoiseLite::NoiseType noiseType;
    float frequency;
    float amplitude;
    int32_t octaves;
    float lacunarity;
    float gain;
    glm::vec3 offset;
    bool invert;
    
    NoiseLayer(FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_Perlin,
               float freq = 0.01f, float amp = 1.0f, int32_t oct = 4)
        : noiseType(type), frequency(freq), amplitude(amp), octaves(oct),
          lacunarity(2.0f), gain(0.5f), offset(0.0f), invert(false) {}
};

/**
 * Multi-layer 3D noise generator for complex terrain
 */
class NoiseGenerator3D {
public:
    NoiseGenerator3D(uint32_t seed = 12345);
    
    void addLayer(const NoiseLayer& layer);
    void clearLayers();
    void setSeed(uint32_t seed);
    
    float sample(float x, float y, float z) const;  // [GPU_CANDIDATE]
    float sampleDensity(const glm::vec3& position, float planetRadius) const;  // [GPU_CANDIDATE]
    
    // Bulk sampling for performance
    void sampleBulk(const std::vector<glm::vec3>& positions, std::vector<float>& results) const;  // [GPU_CANDIDATE]
    
    // Domain warping for more natural terrain
    glm::vec3 domainWarp(const glm::vec3& position, float strength = 1.0f) const;  // [GPU_CANDIDATE]
    
private:
    std::vector<NoiseLayer> layers;
    std::vector<FastNoiseLite> noiseGenerators;
    uint32_t seed;
    
    void updateGenerators();
};

// ============================================================================
// BIOME AND CLIMATE INTEGRATION
// ============================================================================

/**
 * Biome-based material assignment
 */
struct BiomeMaterialMapping {
    uint8_t surfaceMaterial;
    uint8_t subsurfaceMaterial;
    uint8_t deepMaterial;
    float surfaceDepth;
    float subsurfaceDepth;
    
    // Environmental modifiers
    float temperatureMin, temperatureMax;
    float precipitationMin, precipitationMax;
    float elevationMin, elevationMax;
    
    BiomeMaterialMapping(uint8_t surf, uint8_t sub, uint8_t deep)
        : surfaceMaterial(surf), subsurfaceMaterial(sub), deepMaterial(deep),
          surfaceDepth(3.0f), subsurfaceDepth(10.0f),
          temperatureMin(-50.0f), temperatureMax(50.0f),
          precipitationMin(0.0f), precipitationMax(4000.0f),
          elevationMin(-11000.0f), elevationMax(9000.0f) {}
};

/**
 * Climate-driven voxel materialization system
 */
class ClimateVoxelizer {
public:
    ClimateVoxelizer(const MaterialPalette& palette);
    
    void initializeBiomeMappings();
    void setClimateData(const VoxelPlanetData::ClimateLayer& climate);
    
    uint8_t getMaterialForPosition(const glm::vec3& worldPos, float depth, 
                                   const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
    // Bulk material assignment
    void assignMaterialsBulk(const std::vector<glm::vec3>& positions, 
                            const std::vector<float>& depths,
                            std::vector<uint8_t>& materials,
                            const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
private:
    const MaterialPalette& materialPalette;
    std::vector<BiomeMaterialMapping> biomeMappings;
    VoxelPlanetData::ClimateLayer climateData;
    
    // Helper functions
    glm::vec2 worldToClimateCoords(const glm::vec3& worldPos, const VoxelPlanetData& planetData) const;
    float sampleClimateMap(const std::vector<float>& map, int32_t width, int32_t height, 
                          const glm::vec2& coords) const;
    uint8_t getBiomeID(float temperature, float precipitation, float elevation) const;
    uint8_t getMaterialForBiome(uint8_t biomeID, float depth) const;
};

// ============================================================================
// GEOLOGICAL STRATIFICATION
// ============================================================================

/**
 * Geological layer definition
 */
struct GeologicalLayer {
    uint8_t materialID;
    float minDepth;
    float maxDepth;
    float density;          // Probability of occurrence
    float hardnessModifier; // Modify base material hardness
    
    GeologicalLayer(uint8_t mat, float minD, float maxD, float dens = 1.0f)
        : materialID(mat), minDepth(minD), maxDepth(maxD), density(dens), hardnessModifier(1.0f) {}
};

/**
 * Manages geological stratification and mineral deposits
 */
class GeologicalGenerator {
public:
    GeologicalGenerator(uint32_t seed = 54321);
    
    void addLayer(const GeologicalLayer& layer);
    void clearLayers();
    
    uint8_t getGeologicalMaterial(const glm::vec3& worldPos, float surfaceDistance,
                                  const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
    // Mineral deposit generation
    void generateMineralDeposits(const ChunkPos& chunkPos, WorldChunk& chunk,
                                const VoxelPlanetData& planetData);  // [BACKGROUND_THREAD]
    
private:
    std::vector<GeologicalLayer> layers;
    NoiseGenerator3D depositNoise;
    std::mt19937 rng;
    
    // Deposit types
    enum DepositType {
        COAL_SEAM,
        IRON_ORE,
        COPPER_ORE,
        GOLD_VEIN,
        OIL_POCKET,
        GEOTHERMAL_VENT
    };
    
    bool generateDeposit(DepositType type, const glm::vec3& center, float radius,
                        WorldChunk& chunk, const VoxelPlanetData& planetData);
};

// ============================================================================
// MAIN PLANET GENERATOR
// ============================================================================

/**
 * Generates voxel-based planets with 3D density fields
 * Integrates terrain, climate, geology, and caves
 */
class VoxelPlanetGenerator {
public:
    VoxelPlanetGenerator(uint32_t seed = 42);
    ~VoxelPlanetGenerator();
    
    // Planet generation
    void generatePlanet(VoxelPlanetData& planetData, VoxelWorldManager& worldManager);  // [BACKGROUND_THREAD]
    void generateChunk(const ChunkPos& chunkPos, WorldChunk& chunk, 
                      const VoxelPlanetData& planetData);  // [BACKGROUND_THREAD]
    
    // Density field generation
    float calculateDensity(const glm::vec3& worldPos, const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    void generateDensityField(const ChunkPos& chunkPos, std::vector<float>& densityField,
                             const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
    // Material assignment
    void materializeVoxels(const ChunkPos& chunkPos, const std::vector<float>& densityField,
                          WorldChunk& chunk, const VoxelPlanetData& planetData);  // [GPU_CANDIDATE]
    
    // Cave and overhang generation
    void generateCaves(const ChunkPos& chunkPos, std::vector<float>& densityField,
                      const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
    // Configuration
    void setTerrainNoiseConfig(const std::vector<NoiseLayer>& layers);
    void setCaveNoiseConfig(const std::vector<NoiseLayer>& layers);
    void setSeed(uint32_t newSeed);
    
    // Performance settings
    void setGenerationThreadCount(int32_t threadCount);
    void enableGPUAcceleration(bool enable) { useGPU = enable; }
    
    // Statistics
    struct GenerationStats {
        float averageChunkTime;
        int32_t chunksGenerated;
        size_t voxelsGenerated;
        float densityComputeTime;
        float materializationTime;
    };
    GenerationStats getStatistics() const { return stats; }
    
private:
    uint32_t seed;
    bool useGPU;
    
    // Noise generators
    std::unique_ptr<NoiseGenerator3D> terrainNoise;
    std::unique_ptr<NoiseGenerator3D> caveNoise;
    std::unique_ptr<NoiseGenerator3D> detailNoise;
    
    // Specialized generators
    std::unique_ptr<ClimateVoxelizer> climateVoxelizer;
    std::unique_ptr<GeologicalGenerator> geologicalGenerator;
    
    // Threading
    std::vector<std::thread> generationThreads;
    int32_t threadCount;
    
    // Statistics
    mutable GenerationStats stats;
    mutable std::mutex statsMutex;
    
    // Core generation algorithms
    void generateBaseTerrain(const ChunkPos& chunkPos, std::vector<float>& densityField,
                            const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    void applyDetailNoise(const ChunkPos& chunkPos, std::vector<float>& densityField,
                         const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    void generateOverhangs(const ChunkPos& chunkPos, std::vector<float>& densityField,
                          const VoxelPlanetData& planetData) const;  // [GPU_CANDIDATE]
    
    // Helper functions
    glm::vec3 chunkToWorldPosition(const ChunkPos& chunkPos, const VoxelPos& localPos,
                                  const VoxelPlanetData& planetData) const;
    float getDistanceToSurface(const glm::vec3& worldPos, const VoxelPlanetData& planetData) const;
    bool isInsidePlanet(const glm::vec3& worldPos, const VoxelPlanetData& planetData) const;
    
    // Performance optimization
    void optimizeForGPU();
    void optimizeForCPU();
    void updateStatistics(float chunkTime, size_t voxelCount);
}; 