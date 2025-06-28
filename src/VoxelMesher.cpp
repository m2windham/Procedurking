#include "VoxelMesher.h"
#include <unordered_set>
#include <glm/glm.hpp>
#include <algorithm>
#include <memory>

DualContouringMesher::DualContouringMesher()
    : isoValue(0.0f), enableAO(true), useGPU(false), maxVerticesPerChunk(65536) {}
DualContouringMesher::~DualContouringMesher() {}

std::unique_ptr<ChunkMesh> DualContouringMesher::generateMesh(const WorldChunk& chunk, const VoxelPlanetData& planetData) {
    // Minimal stub: create a cube mesh for any solid chunk
    auto mesh = std::make_unique<ChunkMesh>();
    // For now, just fill with a single cube if any voxel is solid
    bool hasSolid = false;
    for (int z = 0; z < WorldChunk::CHUNK_SIZE; ++z)
        for (int y = 0; y < WorldChunk::CHUNK_SIZE; ++y)
            for (int x = 0; x < WorldChunk::CHUNK_SIZE; ++x)
                if (chunk.getVoxel({x, y, z}).materialID != 0) hasSolid = true;
    if (hasSolid) {
        // Add a cube at the chunk center (placeholder)
        glm::vec3 center = glm::vec3(WorldChunk::CHUNK_SIZE / 2);
        float s = WorldChunk::CHUNK_SIZE / 2.0f;
        // 8 vertices
        std::vector<glm::vec3> verts = {
            center + glm::vec3(-s, -s, -s),
            center + glm::vec3( s, -s, -s),
            center + glm::vec3( s,  s, -s),
            center + glm::vec3(-s,  s, -s),
            center + glm::vec3(-s, -s,  s),
            center + glm::vec3( s, -s,  s),
            center + glm::vec3( s,  s,  s),
            center + glm::vec3(-s,  s,  s)
        };
        // 12 triangles (2 per face)
        std::vector<uint32_t> idx = {
            0,1,2, 2,3,0,  // bottom
            4,5,6, 6,7,4,  // top
            0,1,5, 5,4,0,  // front
            2,3,7, 7,6,2,  // back
            1,2,6, 6,5,1,  // right
            3,0,4, 4,7,3   // left
        };
        for (const auto& v : verts) mesh->vertices.push_back(VoxelVertex(v, glm::vec3(0,1,0), 1));
        mesh->indices = idx;
        mesh->triangleCount = 12;
    }
    return mesh;
}

void DualContouringMesher::generateMeshesBatch(const std::vector<std::shared_ptr<WorldChunk>>& chunks, std::vector<std::unique_ptr<ChunkMesh>>& meshes, const VoxelPlanetData& planetData) {
    meshes.clear();
    for (const auto& chunk : chunks) {
        meshes.push_back(generateMesh(*chunk, planetData));
    }
}

std::unique_ptr<LODMesh> DualContouringMesher::generateLODMesh(const WorldChunk& chunk, const VoxelPlanetData& planetData) {
    // Minimal: just use the main mesh for all LODs
    auto lod = std::make_unique<LODMesh>();
    auto mesh = generateMesh(chunk, planetData);
    for (int i = 0; i < 4; ++i) {
        lod->lodLevels[i] = std::make_unique<ChunkMesh>(*mesh);
    }
    return lod;
}

void DualContouringMesher::setEnableGPUAcceleration(bool enable) { useGPU = enable; }

// --- VoxelMeshManager ---

VoxelMeshManager::VoxelMeshManager(const MaterialPalette& palette)
    : materialPalette(palette), maxCachedMeshes(256), maxConcurrentGenerations(2) {
    mesher = std::make_unique<DualContouringMesher>();
}
VoxelMeshManager::~VoxelMeshManager() {}

void VoxelMeshManager::requestMesh(const ChunkPos& chunkPos, std::shared_ptr<WorldChunk> chunk, const VoxelPlanetData& planetData) {
    // Synchronous for now
    meshCache[chunkPos] = mesher->generateMesh(*chunk, planetData);
}

void VoxelMeshManager::requestLODMesh(const ChunkPos& chunkPos, std::shared_ptr<WorldChunk> chunk, const VoxelPlanetData& planetData) {
    lodMeshCache[chunkPos] = mesher->generateLODMesh(*chunk, planetData);
}

std::shared_ptr<ChunkMesh> VoxelMeshManager::getMesh(const ChunkPos& chunkPos) {
    auto it = meshCache.find(chunkPos);
    if (it != meshCache.end()) return it->second;
    return nullptr;
}

std::shared_ptr<LODMesh> VoxelMeshManager::getLODMesh(const ChunkPos& chunkPos) {
    auto it = lodMeshCache.find(chunkPos);
    if (it != lodMeshCache.end()) return it->second;
    return nullptr;
}

void VoxelMeshManager::invalidateMesh(const ChunkPos& chunkPos) {
    meshCache.erase(chunkPos);
    lodMeshCache.erase(chunkPos);
}

void VoxelMeshManager::invalidateRegion(const glm::vec3& center, float radius) {
    // Not implemented
}

void VoxelMeshManager::update(float deltaTime) {}
void VoxelMeshManager::setMaxConcurrentGenerations(int32_t maxConcurrent) { maxConcurrentGenerations = maxConcurrent; }
void VoxelMeshManager::setMaxCachedMeshes(uint32_t maxMeshes) { maxCachedMeshes = maxMeshes; }
void VoxelMeshManager::garbageCollect() {
    // Not implemented
}
size_t VoxelMeshManager::getMemoryUsage() const { return meshCache.size() * sizeof(ChunkMesh); }
std::vector<std::shared_ptr<ChunkMesh>> VoxelMeshManager::getMeshesInFrustum(const glm::mat4& viewProjection) { return {}; }
std::vector<std::shared_ptr<ChunkMesh>> VoxelMeshManager::getMeshesInRadius(const glm::vec3& center, float radius) { return {}; }
VoxelMeshManager::ManagerStats VoxelMeshManager::getStatistics() const { return {}; } 