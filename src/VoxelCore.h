#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <memory>
#include <string>
#include <cmath>

// ============================================================================
// CORE VOXEL DATA STRUCTURES
// ============================================================================

/**
 * The atomic unit of the voxel universe
 * Packed into 32 bits for memory efficiency
 */
struct Voxel {
    uint8_t materialID;        // Index into material palette (256 materials max)
    uint8_t health;           // Damage resistance (0 = destroyed, 255 = indestructible)
    uint8_t flags;            // State bitmask (fire, wet, stressed, etc.)
    uint8_t structuralSupport; // Cached structural integrity value
    
    // Flag bit definitions
    enum Flags : uint8_t {
        IS_ON_FIRE      = 1 << 0,
        IS_WET          = 1 << 1,
        IS_STRESSED     = 1 << 2,
        IS_UNSTABLE     = 1 << 3,
        IS_CONDUCTIVE   = 1 << 4,
        IS_FLAMMABLE    = 1 << 5,
        IS_FLUID        = 1 << 6,
        IS_GROUNDED     = 1 << 7
    };
    
    Voxel() : materialID(0), health(255), flags(0), structuralSupport(255) {}
    Voxel(uint8_t mat, uint8_t hp = 255) : materialID(mat), health(hp), flags(0), structuralSupport(255) {}
    
    bool isDestroyed() const { return health == 0; }
    bool hasFlag(Flags flag) const { return (flags & flag) != 0; }
    void setFlag(Flags flag) { flags |= flag; }
    void clearFlag(Flags flag) { flags &= ~flag; }
};

/**
 * Material properties defining physical and visual characteristics
 */
struct Material {
    std::string name;
    glm::vec3 color;           // Base color
    float roughness;           // Surface roughness (0-1)
    float metallic;            // Metallic property (0-1)
    float emissive;            // Emissive strength
    
    // Physical properties
    float density;             // kg/m³
    float hardness;            // Resistance to damage (0-1)
    float compressionStrength; // Load-bearing capacity
    float tensileStrength;     // Resistance to pulling forces
    float conductivity;        // Heat/electrical conductivity
    float flashPoint;          // Temperature at which material ignites
    float meltingPoint;        // Temperature at which material melts
    
    // Behavior flags
    bool isFlammable;
    bool isLiquid;
    bool isGas;
    bool isTransparent;
    bool isConductive;
    
    Material() = default;
    Material(const std::string& n, glm::vec3 c, float h = 1.0f) 
        : name(n), color(c), roughness(0.5f), metallic(0.0f), emissive(0.0f),
          density(1000.0f), hardness(h), compressionStrength(h), tensileStrength(h * 0.5f),
          conductivity(0.1f), flashPoint(500.0f), meltingPoint(1000.0f),
          isFlammable(false), isLiquid(false), isGas(false), isTransparent(false), isConductive(false) {}
};

/**
 * Material palette for efficient voxel storage
 */
class MaterialPalette {
public:
    MaterialPalette();
    
    uint8_t addMaterial(const Material& material);
    const Material& getMaterial(uint8_t id) const;
    Material& getMaterial(uint8_t id);
    size_t getMaterialCount() const { return materials.size(); }
    
    // Predefined material IDs
    enum StandardMaterials : uint8_t {
        AIR = 0,
        STONE = 1,
        DIRT = 2,
        GRASS = 3,
        SAND = 4,
        WATER = 5,
        WOOD = 6,
        METAL = 7,
        LAVA = 8,
        ICE = 9,
        SNOW = 10,
        COAL = 11,
        OIL = 12,
        CUSTOM_START = 32  // Start of custom materials
    };
    
private:
    std::vector<Material> materials;
    void initializeStandardMaterials();
};

// ============================================================================
// SPATIAL DATA STRUCTURES
// ============================================================================

/**
 * 3D position in voxel space
 */
struct VoxelPos {
    int32_t x, y, z;
    
    VoxelPos() : x(0), y(0), z(0) {}
    VoxelPos(int32_t x_, int32_t y_, int32_t z_) : x(x_), y(y_), z(z_) {}
    
    bool operator==(const VoxelPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    
    VoxelPos operator+(const VoxelPos& other) const {
        return VoxelPos(x + other.x, y + other.y, z + other.z);
    }
    
    VoxelPos operator-(const VoxelPos& other) const {
        return VoxelPos(x - other.x, y - other.y, z - other.z);
    }
    
    float distanceTo(const VoxelPos& other) const {
        float dx = static_cast<float>(x - other.x);
        float dy = static_cast<float>(y - other.y);
        float dz = static_cast<float>(z - other.z);
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

// Hash function for VoxelPos
struct VoxelPosHash {
    size_t operator()(const VoxelPos& pos) const {
        return std::hash<int32_t>()(pos.x) ^ 
               (std::hash<int32_t>()(pos.y) << 1) ^ 
               (std::hash<int32_t>()(pos.z) << 2);
    }
};

/**
 * Chunk coordinates for world partitioning
 */
struct ChunkPos {
    int32_t x, y, z;
    
    ChunkPos() : x(0), y(0), z(0) {}
    ChunkPos(int32_t x_, int32_t y_, int32_t z_) : x(x_), y(y_), z(z_) {}
    ChunkPos(const VoxelPos& voxelPos, int32_t chunkSize) 
        : x(voxelPos.x / chunkSize), y(voxelPos.y / chunkSize), z(voxelPos.z / chunkSize) {}
    
    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
    
    VoxelPos toVoxelPos(int32_t chunkSize) const {
        return VoxelPos(x * chunkSize, y * chunkSize, z * chunkSize);
    }
};

// Hash function for ChunkPos
struct ChunkPosHash {
    size_t operator()(const ChunkPos& pos) const {
        return std::hash<int32_t>()(pos.x) ^ 
               (std::hash<int32_t>()(pos.y) << 1) ^ 
               (std::hash<int32_t>()(pos.z) << 2);
    }
};

// ============================================================================
// WORLD CHUNK DATA
// ============================================================================

/**
 * A cubic section of the voxel world
 * Uses hybrid storage: dense array for active areas, sparse hash for modifications
 */
class WorldChunk {
public:
    static constexpr int32_t CHUNK_SIZE = 64;  // 64x64x64 voxels per chunk
    static constexpr int32_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    
    enum class State {
        UNLOADED,       // Not in memory
        LOADING,        // Being loaded from SVO
        ACTIVE,         // Fully loaded and mutable
        DIRTY_MESH,     // Needs mesh regeneration
        DIRTY_PHYSICS,  // Needs physics update
        DIRTY_STRUCTURE // Needs structural analysis
    };
    
    WorldChunk(const ChunkPos& position);
    ~WorldChunk();
    
    // Voxel access
    const Voxel& getVoxel(const VoxelPos& localPos) const;
    void setVoxel(const VoxelPos& localPos, const Voxel& voxel);
    bool hasVoxel(const VoxelPos& localPos) const;
    
    // Chunk management
    ChunkPos getPosition() const { return position; }
    State getState() const { return state; }
    void setState(State newState) { state = newState; }
    
    // Modification tracking
    bool isDirty() const { return state >= State::DIRTY_MESH; }
    bool needsMeshUpdate() const { return state == State::DIRTY_MESH || state == State::DIRTY_PHYSICS; }
    bool needsPhysicsUpdate() const { return state == State::DIRTY_PHYSICS || state == State::DIRTY_STRUCTURE; }
    bool needsStructuralAnalysis() const { return state == State::DIRTY_STRUCTURE; }
    
    // Memory management
    void compress();    // Convert to sparse representation
    void decompress();  // Convert to dense representation
    size_t getMemoryUsage() const;
    
    // Neighbor access for algorithms
    std::array<VoxelPos, 6> getNeighborPositions(const VoxelPos& localPos) const;
    std::array<VoxelPos, 26> getExtendedNeighborPositions(const VoxelPos& localPos) const;
    
private:
    ChunkPos position;
    State state;
    
    // Hybrid storage system
    std::unique_ptr<std::array<Voxel, CHUNK_VOLUME>> denseStorage;  // For active chunks
    std::unique_ptr<std::unordered_map<VoxelPos, Voxel, VoxelPosHash>> sparseStorage;  // For sparse chunks
    
    bool isDense;
    
    // Helper functions
    int32_t getIndex(const VoxelPos& localPos) const;
    VoxelPos getLocalPos(int32_t index) const;
    bool isValidLocalPos(const VoxelPos& localPos) const;
};

// ============================================================================
// PLANET DATA STRUCTURE
// ============================================================================

/**
 * Extended planet data for voxel-based worlds
 */
struct VoxelPlanetData {
    // Original planet data
    float mass;
    float radius;
    float gravity;
    float axialTilt;
    float rotationPeriod;
    
    // Voxel-specific data
    float voxelSize;           // Size of each voxel in world units
    int32_t planetRadiusVoxels; // Planet radius in voxel units
    glm::vec3 planetCenter;    // Center position in world space
    
    // Generation parameters
    float noiseScale;
    float noiseAmplitude;
    int32_t noiseOctaves;
    float caveThreshold;
    float surfaceThreshold;
    
    // Climate data (from original system)
    struct ClimateLayer {
        std::vector<float> temperatureMap;
        std::vector<float> precipitationMap;
        std::vector<uint8_t> biomeMap;
        int32_t mapWidth;
        int32_t mapHeight;
    } climate;
    
    VoxelPlanetData() 
        : mass(1.0f), radius(6371000.0f), gravity(9.81f), axialTilt(23.5f), rotationPeriod(24.0f),
          voxelSize(1.0f), planetRadiusVoxels(6371000), planetCenter(0.0f),
          noiseScale(0.01f), noiseAmplitude(0.3f), noiseOctaves(6), 
          caveThreshold(-0.1f), surfaceThreshold(0.0f) {}
};

// ============================================================================
// DESTRUCTION AND PHYSICS DATA
// ============================================================================

/**
 * Data for tracking destruction events and debris
 */
struct DestructionEvent {
    VoxelPos epicenter;
    float radius;
    float damage;
    float timestamp;
    glm::vec3 velocity;  // For directional explosions
    
    enum Type {
        EXPLOSION,
        IMPACT,
        STRUCTURAL_COLLAPSE,
        FIRE_DAMAGE,
        ACID_CORROSION
    } type;
};

/**
 * Debris object created from destroyed voxel clusters
 */
struct DebrisObject {
    uint32_t id;
    std::vector<VoxelPos> voxelPositions;
    glm::vec3 centerOfMass;
    glm::vec3 velocity;
    glm::vec3 angularVelocity;
    float mass;
    bool isStatic;
    
    // Physics integration
    glm::mat4 transform;
    glm::vec3 forces;
    glm::vec3 torques;
};

/**
 * Structural integrity data for voxels
 */
struct StructuralData {
    float supportValue;    // How much support this voxel provides
    float loadValue;       // How much load this voxel carries
    float stressRatio;     // load / support (>1.0 = failure)
    bool isGrounded;       // Connected to bedrock/foundation
    int32_t supportChain;  // Distance to nearest grounded voxel
}; 