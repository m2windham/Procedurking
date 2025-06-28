#include "VoxelPlanetGenerator.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>

VoxelPlanetGenerator::VoxelPlanetGenerator(uint32_t seed)
    : seed(seed), useGPU(false), threadCount(4) {
    terrainNoise = std::make_unique<NoiseGenerator3D>(seed);
    caveNoise = std::make_unique<NoiseGenerator3D>(seed + 1);
    detailNoise = std::make_unique<NoiseGenerator3D>(seed + 2);
    climateVoxelizer = std::make_unique<ClimateVoxelizer>(MaterialPalette());
    geologicalGenerator = std::make_unique<GeologicalGenerator>(seed + 3);
}

VoxelPlanetGenerator::~VoxelPlanetGenerator() {}

void VoxelPlanetGenerator::generatePlanet(VoxelPlanetData& planetData, VoxelWorldManager& worldManager) {
    // Example: generate a single chunk at the planet center for now
    ChunkPos centerChunk(0, 0, 0);
    auto chunk = std::make_shared<WorldChunk>(centerChunk);
    generateChunk(centerChunk, *chunk, planetData);
    worldManager.setVoxelBulk({}); // Placeholder for now
}

void VoxelPlanetGenerator::generateChunk(const ChunkPos& chunkPos, WorldChunk& chunk, const VoxelPlanetData& planetData) {
    std::vector<float> densityField(WorldChunk::CHUNK_VOLUME, 0.0f);
    generateDensityField(chunkPos, densityField, planetData);
    materializeVoxels(chunkPos, densityField, chunk, planetData);
}

float VoxelPlanetGenerator::calculateDensity(const glm::vec3& worldPos, const VoxelPlanetData& planetData) const {
    // Simple sphere: inside = solid, outside = air
    float dist = glm::length(worldPos - planetData.planetCenter);
    float surface = planetData.radius;
    return surface - dist;
}

void VoxelPlanetGenerator::generateDensityField(const ChunkPos& chunkPos, std::vector<float>& densityField, const VoxelPlanetData& planetData) const {
    for (int z = 0; z < WorldChunk::CHUNK_SIZE; ++z) {
        for (int y = 0; y < WorldChunk::CHUNK_SIZE; ++y) {
            for (int x = 0; x < WorldChunk::CHUNK_SIZE; ++x) {
                int idx = x + y * WorldChunk::CHUNK_SIZE + z * WorldChunk::CHUNK_SIZE * WorldChunk::CHUNK_SIZE;
                glm::vec3 worldPos = chunkToWorldPosition(chunkPos, {x, y, z}, planetData);
                densityField[idx] = calculateDensity(worldPos, planetData);
            }
        }
    }
}

void VoxelPlanetGenerator::materializeVoxels(const ChunkPos& chunkPos, const std::vector<float>& densityField, WorldChunk& chunk, const VoxelPlanetData& planetData) {
    for (int z = 0; z < WorldChunk::CHUNK_SIZE; ++z) {
        for (int y = 0; y < WorldChunk::CHUNK_SIZE; ++y) {
            for (int x = 0; x < WorldChunk::CHUNK_SIZE; ++x) {
                int idx = x + y * WorldChunk::CHUNK_SIZE + z * WorldChunk::CHUNK_SIZE * WorldChunk::CHUNK_SIZE;
                if (densityField[idx] > 0.0f) {
                    chunk.setVoxel({x, y, z}, Voxel(1, 255)); // 1 = stone for now
                } else {
                    chunk.setVoxel({x, y, z}, Voxel(0, 0)); // 0 = air
                }
            }
        }
    }
}

void VoxelPlanetGenerator::generateCaves(const ChunkPos& chunkPos, std::vector<float>& densityField, const VoxelPlanetData& planetData) const {
    // TODO: Add cave noise
}

void VoxelPlanetGenerator::setTerrainNoiseConfig(const std::vector<NoiseLayer>& layers) {
    terrainNoise->clearLayers();
    for (const auto& layer : layers) terrainNoise->addLayer(layer);
}

void VoxelPlanetGenerator::setCaveNoiseConfig(const std::vector<NoiseLayer>& layers) {
    caveNoise->clearLayers();
    for (const auto& layer : layers) caveNoise->addLayer(layer);
}

void VoxelPlanetGenerator::setSeed(uint32_t newSeed) {
    seed = newSeed;
    terrainNoise->setSeed(seed);
    caveNoise->setSeed(seed + 1);
    detailNoise->setSeed(seed + 2);
}

void VoxelPlanetGenerator::setGenerationThreadCount(int32_t count) {
    threadCount = count;
}

void VoxelPlanetGenerator::optimizeForGPU() { useGPU = true; }
void VoxelPlanetGenerator::optimizeForCPU() { useGPU = false; }

VoxelPlanetGenerator::GenerationStats VoxelPlanetGenerator::getStatistics() const { return stats; }

// Helper
glm::vec3 VoxelPlanetGenerator::chunkToWorldPosition(const ChunkPos& chunkPos, const VoxelPos& localPos, const VoxelPlanetData& planetData) const {
    return glm::vec3(
        (chunkPos.x * WorldChunk::CHUNK_SIZE + localPos.x) * planetData.voxelSize,
        (chunkPos.y * WorldChunk::CHUNK_SIZE + localPos.y) * planetData.voxelSize,
        (chunkPos.z * WorldChunk::CHUNK_SIZE + localPos.z) * planetData.voxelSize
    );
} 