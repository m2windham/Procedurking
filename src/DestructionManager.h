#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include <queue>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

// ============================================================================
// DESTRUCTION ALGORITHMS
// ============================================================================

/**
 * Impact damage calculation parameters
 */
struct ImpactParameters {
    glm::vec3 epicenter;
    float radius;
    float maxDamage;
    float falloffExponent;
    glm::vec3 direction;      // For directional explosions
    float directionalBias;    // 0.0 = spherical, 1.0 = fully directional
    
    enum DamageType {
        EXPLOSIVE,            // Spherical blast
        PROJECTILE,          // Directional impact
        SEISMIC,             // Ground shockwave
        THERMAL,             // Heat-based damage
        CHEMICAL,            // Acid/corrosion
        ELECTROMAGNETIC      // EMP/energy weapon
    } damageType;
    
    float penetrationDepth;   // How deep damage penetrates
    bool causesChainReaction; // Can trigger secondary explosions
    
    ImpactParameters(const glm::vec3& center, float rad, float damage)
        : epicenter(center), radius(rad), maxDamage(damage), falloffExponent(2.0f),
          direction(0.0f, -1.0f, 0.0f), directionalBias(0.0f), damageType(EXPLOSIVE),
          penetrationDepth(rad * 0.5f), causesChainReaction(false) {}
};

/**
 * Connectivity cluster data for debris generation
 */
struct VoxelCluster {
    uint32_t clusterID;
    std::vector<VoxelPos> voxelPositions;
    glm::vec3 centerOfMass;
    float totalMass;
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    bool isGrounded;          // Connected to immovable foundation
    uint8_t dominantMaterial;
    
    VoxelCluster(uint32_t id) : clusterID(id), totalMass(0.0f), isGrounded(false), dominantMaterial(0) {}
    
    void calculateProperties(const MaterialPalette& palette);
    bool shouldBecomeDebris() const;
    float getStabilityScore() const;
};

/**
 * Real-time destruction and debris physics manager
 */
class DestructionManager {
public:
    DestructionManager(VoxelWorldManager& worldManager, const MaterialPalette& palette);
    ~DestructionManager();
    
    // Primary destruction interface
    void applyDestruction(const ImpactParameters& impact);
    void applyExplosion(const glm::vec3& center, float radius, float damage);
    void applyProjectileImpact(const glm::vec3& start, const glm::vec3& end, 
                              float radius, float damage);
    
    // Continuous damage effects
    void applyFireDamage(const glm::vec3& center, float radius, float deltaTime);
    void applyAcidCorrosion(const glm::vec3& center, float radius, float deltaTime);
    void applySeismicDamage(const glm::vec3& epicenter, float magnitude, float deltaTime);
    
    // Update and processing
    void update(float deltaTime);  // Process destruction queue and debris physics
    
    // Debris management
    std::vector<std::shared_ptr<DebrisObject>> getDebrisInRadius(const glm::vec3& center, float radius);
    void removeDebris(uint32_t debrisID);
    void clearAllDebris();
    
    // Chain reaction system
    void enableChainReactions(bool enable) { enableChainReactions = enable; }
    void setChainReactionThreshold(float threshold) { chainReactionThreshold = threshold; }
    
    // Performance settings
    void setMaxConcurrentDestructions(int32_t maxDestructions);
    void setMaxDebrisObjects(uint32_t maxDebris);
    void setDebrisLifetime(float lifetime) { debrisLifetime = lifetime; }
    
    // Statistics
    struct DestructionStats {
        uint32_t voxelsDestroyed;
        uint32_t debrisObjectsCreated;
        uint32_t chainReactionsTriggered;
        float averageClusterAnalysisTime;
        uint32_t activeDebrisObjects;
    };
    DestructionStats getStatistics() const { return stats; }
    
private:
    VoxelWorldManager& worldManager;
    const MaterialPalette& materialPalette;
    
    // Destruction processing
    struct DestructionTask {
        ImpactParameters impact;
        float timestamp;
        uint32_t priority;
        
        DestructionTask(const ImpactParameters& imp, float time, uint32_t pri = 0)
            : impact(imp), timestamp(time), priority(pri) {}
    };
    
    std::priority_queue<DestructionTask> destructionQueue;
    std::vector<std::thread> destructionThreads;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldStop;
    
    // Debris tracking
    std::unordered_map<uint32_t, std::shared_ptr<DebrisObject>> activeDebris;
    uint32_t nextDebrisID;
    std::mutex debrisMutex;
    
    // Chain reaction system
    bool enableChainReactions;
    float chainReactionThreshold;
    std::queue<glm::vec3> pendingChainReactions;
    
    // Performance limits
    int32_t maxConcurrentDestructions;
    uint32_t maxDebrisObjects;
    float debrisLifetime;
    
    // Statistics
    mutable DestructionStats stats;
    mutable std::mutex statsMutex;
    
    // Core destruction algorithms
    void processDestruction(const ImpactParameters& impact);  // [BACKGROUND_THREAD]
    void applyVoxelDamage(const VoxelPos& pos, float damage, 
                         const ImpactParameters& impact);  // [GPU_CANDIDATE]
    void performConnectivityAnalysis(const std::unordered_set<ChunkPos, ChunkPosHash>& affectedChunks);  // [BACKGROUND_THREAD]
    
    // Connectivity analysis (3D Flood Fill)
    std::vector<VoxelCluster> findDisconnectedClusters(const ChunkPos& chunkPos);  // [BACKGROUND_THREAD]
    void floodFillCluster(const VoxelPos& startPos, VoxelCluster& cluster,
                         std::unordered_set<VoxelPos, VoxelPosHash>& visited);  // [BACKGROUND_THREAD]
    bool isVoxelGrounded(const VoxelPos& pos);
    
    // Debris creation and physics
    std::shared_ptr<DebrisObject> createDebrisFromCluster(const VoxelCluster& cluster);
    void updateDebrisPhysics(float deltaTime);  // [BACKGROUND_THREAD]
    void applyGravityToDebris(DebrisObject& debris, float deltaTime);
    void handleDebrisCollisions();
    
    // Damage calculation
    float calculateDamageAtPosition(const glm::vec3& position, const ImpactParameters& impact);
    float getDistanceAttenuation(float distance, float radius, float exponent);
    float getDirectionalAttenuation(const glm::vec3& position, const ImpactParameters& impact);
    float getMaterialResistance(uint8_t materialID, ImpactParameters::DamageType damageType);
    
    // Chain reaction processing
    void checkForChainReactions(const VoxelPos& pos, const ImpactParameters& originalImpact);
    void triggerChainReaction(const glm::vec3& position, float magnitude);
    bool isMaterialExplosive(uint8_t materialID);
    
    // Shockwave propagation
    void propagateShockwave(const glm::vec3& epicenter, float magnitude, float speed);  // [GPU_CANDIDATE]
    void updateShockwaves(float deltaTime);
    
    // Worker thread functions
    void destructionWorker();  // [BACKGROUND_THREAD]
    void debrisPhysicsWorker();  // [BACKGROUND_THREAD]
    
    // Memory management
    void cleanupExpiredDebris();
    void optimizeDebrisStorage();
    
    // Helper functions
    std::unordered_set<ChunkPos, ChunkPosHash> getAffectedChunks(const ImpactParameters& impact);
    std::vector<VoxelPos> getVoxelsInRadius(const glm::vec3& center, float radius);
    bool isVoxelInRadius(const VoxelPos& pos, const glm::vec3& center, float radius);
    float calculateVoxelMass(const VoxelPos& pos);
    
    // Optimization for large-scale destruction
    void batchProcessVoxelDamage(const std::vector<std::pair<VoxelPos, float>>& damageList,
                                const ImpactParameters& impact);  // [GPU_CANDIDATE]
    void parallelConnectivityAnalysis(const std::vector<ChunkPos>& chunks);  // [BACKGROUND_THREAD]
};

// ============================================================================
// SPECIALIZED DESTRUCTION EFFECTS
// ============================================================================

/**
 * Specialized destruction effects for different scenarios
 */
class SpecializedDestruction {
public:
    SpecializedDestruction(DestructionManager& destructionMgr);
    
    // Geological events
    void triggerEarthquake(const glm::vec3& epicenter, float magnitude, float duration);
    void createSinkhole(const glm::vec3& center, float radius, float depth);
    void simulateVolcanicEruption(const glm::vec3& vent, float intensity);
    
    // Weapon systems
    void nuclearExplosion(const glm::vec3& groundZero, float yield);  // Kilotons TNT equivalent
    void kineticImpactor(const glm::vec3& impactPoint, const glm::vec3& velocity, float mass);
    void laserBeam(const glm::vec3& start, const glm::vec3& end, float power, float duration);
    
    // Environmental destruction
    void meteoriteImpact(const glm::vec3& impactPoint, float diameter, float velocity);
    void tsunamiWave(const glm::vec3& origin, float height, const glm::vec3& direction);
    void avalanche(const glm::vec3& triggerPoint, const glm::vec3& direction, float volume);
    
    // Industrial accidents
    void chemicalExplosion(const glm::vec3& center, float radius, bool toxic);
    void structuralCollapse(const glm::vec3& weakPoint, float area);
    void damBreak(const glm::vec3& damCenter, const glm::vec3& flowDirection);
    
private:
    DestructionManager& destructionManager;
    
    // Helper functions for complex destruction patterns
    void createCraterPattern(const glm::vec3& center, float radius, float depth);
    void createShockwavePattern(const glm::vec3& epicenter, float speed, float duration);
    void createDebrisField(const glm::vec3& center, float radius, float velocity);
}; 