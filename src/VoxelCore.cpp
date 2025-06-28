#include "VoxelCore.h"
#include <algorithm>
#include <stdexcept>

// ============================================================================
// MATERIAL PALETTE IMPLEMENTATION
// ============================================================================

MaterialPalette::MaterialPalette() {
    materials.reserve(256);
    initializeStandardMaterials();
}

uint8_t MaterialPalette::addMaterial(const Material& material) {
    if (materials.size() >= 255) {
        throw std::runtime_error("Material palette is full (max 255 materials)");
    }
    
    materials.push_back(material);
    return static_cast<uint8_t>(materials.size() - 1);
}

const Material& MaterialPalette::getMaterial(uint8_t id) const {
    if (id >= materials.size()) {
        throw std::out_of_range("Material ID out of range");
    }
    return materials[id];
}

Material& MaterialPalette::getMaterial(uint8_t id) {
    if (id >= materials.size()) {
        throw std::out_of_range("Material ID out of range");
    }
    return materials[id];
}

void MaterialPalette::initializeStandardMaterials() {
    // AIR (ID 0) - Empty space
    Material air("Air", glm::vec3(0.0f, 0.0f, 0.0f), 0.0f);
    air.density = 1.225f; // kg/m³ at sea level
    air.isGas = true;
    air.isTransparent = true;
    materials.push_back(air);
    
    // STONE (ID 1) - Basic rock
    Material stone("Stone", glm::vec3(0.5f, 0.5f, 0.5f), 0.8f);
    stone.density = 2700.0f;
    stone.compressionStrength = 0.8f;
    stone.tensileStrength = 0.4f;
    materials.push_back(stone);
    
    // DIRT (ID 2) - Soil
    Material dirt("Dirt", glm::vec3(0.4f, 0.3f, 0.2f), 0.3f);
    dirt.density = 1500.0f;
    dirt.compressionStrength = 0.3f;
    dirt.tensileStrength = 0.1f;
    materials.push_back(dirt);
    
    // GRASS (ID 3) - Surface vegetation
    Material grass("Grass", glm::vec3(0.2f, 0.7f, 0.2f), 0.2f);
    grass.density = 800.0f;
    grass.isFlammable = true;
    grass.flashPoint = 250.0f;
    materials.push_back(grass);
    
    // SAND (ID 4) - Granular material
    Material sand("Sand", glm::vec3(0.8f, 0.7f, 0.5f), 0.4f);
    sand.density = 1600.0f;
    sand.compressionStrength = 0.2f;
    sand.tensileStrength = 0.05f;
    materials.push_back(sand);
    
    // WATER (ID 5) - Liquid
    Material water("Water", glm::vec3(0.2f, 0.4f, 0.8f), 0.0f);
    water.density = 1000.0f;
    water.isLiquid = true;
    water.isTransparent = true;
    water.conductivity = 0.6f;
    materials.push_back(water);
    
    // WOOD (ID 6) - Organic solid
    Material wood("Wood", glm::vec3(0.6f, 0.4f, 0.2f), 0.5f);
    wood.density = 600.0f;
    wood.isFlammable = true;
    wood.flashPoint = 300.0f;
    wood.compressionStrength = 0.5f;
    wood.tensileStrength = 0.3f;
    materials.push_back(wood);
    
    // METAL (ID 7) - Iron/Steel
    Material metal("Metal", glm::vec3(0.7f, 0.7f, 0.7f), 0.9f);
    metal.density = 7850.0f;
    metal.metallic = 1.0f;
    metal.isConductive = true;
    metal.conductivity = 80.0f;
    metal.compressionStrength = 0.9f;
    metal.tensileStrength = 0.8f;
    metal.meltingPoint = 1538.0f;
    materials.push_back(metal);
    
    // LAVA (ID 8) - Molten rock
    Material lava("Lava", glm::vec3(1.0f, 0.3f, 0.0f), 0.7f);
    lava.density = 2800.0f;
    lava.emissive = 1.0f;
    lava.isLiquid = true;
    lava.flashPoint = 0.0f; // Always hot enough to ignite
    materials.push_back(lava);
    
    // ICE (ID 9) - Frozen water
    Material ice("Ice", glm::vec3(0.8f, 0.9f, 1.0f), 0.6f);
    ice.density = 917.0f;
    ice.isTransparent = true;
    ice.meltingPoint = 0.0f;
    ice.compressionStrength = 0.4f;
    ice.tensileStrength = 0.2f;
    materials.push_back(ice);
    
    // SNOW (ID 10) - Crystalline ice
    Material snow("Snow", glm::vec3(0.95f, 0.95f, 0.95f), 0.1f);
    snow.density = 300.0f;
    snow.meltingPoint = 0.0f;
    snow.compressionStrength = 0.1f;
    snow.tensileStrength = 0.05f;
    materials.push_back(snow);
    
    // COAL (ID 11) - Combustible carbon
    Material coal("Coal", glm::vec3(0.1f, 0.1f, 0.1f), 0.7f);
    coal.density = 1300.0f;
    coal.isFlammable = true;
    coal.flashPoint = 200.0f;
    coal.compressionStrength = 0.6f;
    materials.push_back(coal);
    
    // OIL (ID 12) - Liquid hydrocarbon
    Material oil("Oil", glm::vec3(0.2f, 0.2f, 0.1f), 0.3f);
    oil.density = 850.0f;
    oil.isLiquid = true;
    oil.isFlammable = true;
    oil.flashPoint = 150.0f;
    materials.push_back(oil);
}

// ============================================================================
// WORLD CHUNK IMPLEMENTATION
// ============================================================================

WorldChunk::WorldChunk(const ChunkPos& position) 
    : position(position), state(State::UNLOADED), isDense(false) {
    // Start with sparse storage
    sparseStorage = std::make_unique<std::unordered_map<VoxelPos, Voxel, VoxelPosHash>>();
}

WorldChunk::~WorldChunk() = default;

const Voxel& WorldChunk::getVoxel(const VoxelPos& localPos) const {
    if (!isValidLocalPos(localPos)) {
        static const Voxel airVoxel(MaterialPalette::AIR, 0);
        return airVoxel;
    }
    
    if (isDense && denseStorage) {
        int32_t index = getIndex(localPos);
        return (*denseStorage)[index];
    } else if (sparseStorage) {
        auto it = sparseStorage->find(localPos);
        if (it != sparseStorage->end()) {
            return it->second;
        }
    }
    
    static const Voxel airVoxel(MaterialPalette::AIR, 0);
    return airVoxel;
}

void WorldChunk::setVoxel(const VoxelPos& localPos, const Voxel& voxel) {
    if (!isValidLocalPos(localPos)) {
        return;
    }
    
    if (isDense && denseStorage) {
        int32_t index = getIndex(localPos);
        (*denseStorage)[index] = voxel;
    } else if (sparseStorage) {
        if (voxel.materialID == MaterialPalette::AIR) {
            // Remove air voxels from sparse storage
            sparseStorage->erase(localPos);
        } else {
            (*sparseStorage)[localPos] = voxel;
        }
    }
    
    // Mark chunk as dirty
    if (state == State::ACTIVE) {
        state = State::DIRTY_MESH;
    }
}

bool WorldChunk::hasVoxel(const VoxelPos& localPos) const {
    const Voxel& voxel = getVoxel(localPos);
    return voxel.materialID != MaterialPalette::AIR;
}

void WorldChunk::compress() {
    if (!isDense || !denseStorage) {
        return; // Already compressed or no data
    }
    
    // Convert dense to sparse storage
    auto newSparseStorage = std::make_unique<std::unordered_map<VoxelPos, Voxel, VoxelPosHash>>();
    
    for (int32_t x = 0; x < CHUNK_SIZE; ++x) {
        for (int32_t y = 0; y < CHUNK_SIZE; ++y) {
            for (int32_t z = 0; z < CHUNK_SIZE; ++z) {
                VoxelPos localPos(x, y, z);
                int32_t index = getIndex(localPos);
                const Voxel& voxel = (*denseStorage)[index];
                
                if (voxel.materialID != MaterialPalette::AIR) {
                    (*newSparseStorage)[localPos] = voxel;
                }
            }
        }
    }
    
    denseStorage.reset();
    sparseStorage = std::move(newSparseStorage);
    isDense = false;
}

void WorldChunk::decompress() {
    if (isDense || !sparseStorage) {
        return; // Already decompressed or no data
    }
    
    // Convert sparse to dense storage
    auto newDenseStorage = std::make_unique<std::array<Voxel, CHUNK_VOLUME>>();
    
    // Initialize all voxels to air
    for (auto& voxel : *newDenseStorage) {
        voxel = Voxel(MaterialPalette::AIR, 0);
    }
    
    // Copy sparse data to dense storage
    for (const auto& [pos, voxel] : *sparseStorage) {
        if (isValidLocalPos(pos)) {
            int32_t index = getIndex(pos);
            (*newDenseStorage)[index] = voxel;
        }
    }
    
    sparseStorage.reset();
    denseStorage = std::move(newDenseStorage);
    isDense = true;
}

size_t WorldChunk::getMemoryUsage() const {
    size_t usage = sizeof(WorldChunk);
    
    if (isDense && denseStorage) {
        usage += sizeof(std::array<Voxel, CHUNK_VOLUME>);
    } else if (sparseStorage) {
        usage += sizeof(std::unordered_map<VoxelPos, Voxel, VoxelPosHash>);
        usage += sparseStorage->size() * (sizeof(VoxelPos) + sizeof(Voxel));
    }
    
    return usage;
}

std::array<VoxelPos, 6> WorldChunk::getNeighborPositions(const VoxelPos& localPos) const {
    return {{
        VoxelPos(localPos.x + 1, localPos.y, localPos.z),
        VoxelPos(localPos.x - 1, localPos.y, localPos.z),
        VoxelPos(localPos.x, localPos.y + 1, localPos.z),
        VoxelPos(localPos.x, localPos.y - 1, localPos.z),
        VoxelPos(localPos.x, localPos.y, localPos.z + 1),
        VoxelPos(localPos.x, localPos.y, localPos.z - 1)
    }};
}

std::array<VoxelPos, 26> WorldChunk::getExtendedNeighborPositions(const VoxelPos& localPos) const {
    std::array<VoxelPos, 26> neighbors;
    int32_t index = 0;
    
    for (int32_t dx = -1; dx <= 1; ++dx) {
        for (int32_t dy = -1; dy <= 1; ++dy) {
            for (int32_t dz = -1; dz <= 1; ++dz) {
                if (dx == 0 && dy == 0 && dz == 0) {
                    continue; // Skip center voxel
                }
                
                neighbors[index++] = VoxelPos(
                    localPos.x + dx,
                    localPos.y + dy,
                    localPos.z + dz
                );
            }
        }
    }
    
    return neighbors;
}

int32_t WorldChunk::getIndex(const VoxelPos& localPos) const {
    return localPos.x + localPos.y * CHUNK_SIZE + localPos.z * CHUNK_SIZE * CHUNK_SIZE;
}

VoxelPos WorldChunk::getLocalPos(int32_t index) const {
    int32_t z = index / (CHUNK_SIZE * CHUNK_SIZE);
    index %= (CHUNK_SIZE * CHUNK_SIZE);
    int32_t y = index / CHUNK_SIZE;
    int32_t x = index % CHUNK_SIZE;
    return VoxelPos(x, y, z);
}

bool WorldChunk::isValidLocalPos(const VoxelPos& localPos) const {
    return localPos.x >= 0 && localPos.x < CHUNK_SIZE &&
           localPos.y >= 0 && localPos.y < CHUNK_SIZE &&
           localPos.z >= 0 && localPos.z < CHUNK_SIZE;
} 