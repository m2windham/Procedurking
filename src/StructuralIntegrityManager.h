#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <climits>

// ============================================================================
// STRUCTURAL ANALYSIS DATA
// ============================================================================

/**
 * Stress analysis parameters for materials
 */
struct MaterialStressProperties {
    float compressionStrength;    // Maximum compression load (N/m²)
    float tensileStrength;       // Maximum tension load (N/m²)
    float shearStrength;         // Maximum shear load (N/m²)
    float elasticModulus;        // Young's modulus (Pa)
    float poissonRatio;          // Poisson's ratio (0-0.5)
    float fatigueLimit;          // Stress limit for infinite cycles
    float densityKgM3;           // Material density (kg/m³)
    
    // Failure characteristics
    bool isBrittle;              // True for brittle materials (concrete, glass)
    float ductilityFactor;       // How much material can deform before failure
    float crackPropagationSpeed; // How fast cracks spread through material
    
    MaterialStressProperties()
        : compressionStrength(50e6f), tensileStrength(25e6f), shearStrength(20e6f),
          elasticModulus(200e9f), poissonRatio(0.3f), fatigueLimit(100e6f), densityKgM3(2500.0f),
          isBrittle(false), ductilityFactor(0.1f), crackPropagationSpeed(1000.0f) {}
};

/**
 * Stress state for a single voxel
 */
struct VoxelStressState {
    glm::vec3 normalStress;      // Normal stress in X, Y, Z directions (Pa)
    glm::vec3 shearStress;       // Shear stress components (Pa)
    float vonMisesStress;        // Equivalent stress (Pa)
    float hydrostaticPressure;   // Pressure component (Pa)
    
    // Support and load values
    float supportValue;          // How much support this voxel provides
    float loadValue;             // How much load this voxel carries
    float stressRatio;           // loadValue / supportValue (>1.0 = failure)
    
    // Structural connectivity
    bool isGrounded;             // Connected to bedrock/foundation
    int32_t supportChain;        // Distance to nearest grounded voxel
    uint8_t supportDirection;    // Bitmask of directions providing support
    
    // Failure tracking
    bool isUnderStress;          // True if stress > threshold
    bool isCracked;              // True if micro-cracks have formed
    bool hasFailureRisk;         // True if close to failure
    float fatigueAccumulation;   // Accumulated fatigue damage (0-1)
    
    VoxelStressState()
        : normalStress(0.0f), shearStress(0.0f), vonMisesStress(0.0f), hydrostaticPressure(0.0f),
          supportValue(0.0f), loadValue(0.0f), stressRatio(0.0f),
          isGrounded(false), supportChain(INT32_MAX), supportDirection(0),
          isUnderStress(false), isCracked(false), hasFailureRisk(false), fatigueAccumulation(0.0f) {}
};

/**
 * Chunk-level structural analysis data
 */
struct ChunkStructuralData {
    ChunkPos position;
    std::array<VoxelStressState, WorldChunk::CHUNK_VOLUME> stressStates;
    
    // Analysis metadata
    bool needsAnalysis;          // True if chunk needs stress recalculation
    float lastAnalysisTime;      // Timestamp of last analysis
    float analysisComplexity;    // Computational cost estimate
    uint32_t failureCount;       // Number of failed voxels in chunk
    
    // Optimization data
    bool isStable;               // True if no significant stress changes
    float stabilityScore;        // 0.0 = highly unstable, 1.0 = completely stable
    glm::vec3 centerOfMass;      // Center of mass for the chunk
    
    ChunkStructuralData(const ChunkPos& pos)
        : position(pos), needsAnalysis(true), lastAnalysisTime(0.0f),
          analysisComplexity(1.0f), failureCount(0), isStable(false),
          stabilityScore(0.5f), centerOfMass(0.0f) {}
    
    VoxelStressState& getStressState(const VoxelPos& localPos);
    const VoxelStressState& getStressState(const VoxelPos& localPos) const;
    void markForAnalysis() { needsAnalysis = true; isStable = false; }
};

// ============================================================================
// STRUCTURAL INTEGRITY MANAGER
// ============================================================================

/**
 * Asynchronous structural integrity simulation
 * Implements simplified FEA for real-time structural collapse
 */
class StructuralIntegrityManager {
public:
    StructuralIntegrityManager(VoxelWorldManager& worldManager, const MaterialPalette& palette);
    ~StructuralIntegrityManager();
    
    // Primary interface
    void markChunkForAnalysis(const ChunkPos& chunkPos);
    void markRegionForAnalysis(const glm::vec3& center, float radius);
    void update(float deltaTime);  // Process analysis queue
    
    // Stress query interface
    VoxelStressState getVoxelStress(const VoxelPos& pos) const;
    float getStructuralStability(const ChunkPos& chunkPos) const;
    std::vector<VoxelPos> getFailureRiskVoxels(const ChunkPos& chunkPos) const;
    
    // Failure simulation
    void simulateStructuralFailure(const VoxelPos& triggerPos);
    void propagateStructuralDamage(const VoxelPos& failurePos, float magnitude);
    
    // Configuration
    void setAnalysisThreadCount(int32_t threadCount);
    void setMaxAnalysisQueueSize(uint32_t maxSize);
    void setStabilityThreshold(float threshold) { stabilityThreshold = threshold; }
    void setGravityVector(const glm::vec3& gravity) { gravityVector = gravity; }
    
    // Performance tuning
    void setAnalysisFrequency(float frequency) { analysisFrequency = frequency; }
    void setEnableDetailedAnalysis(bool enable) { enableDetailedAnalysis = enable; }
    void setEnableFatigueSimulation(bool enable) { enableFatigueSimulation = enable; }
    
    // Statistics
    struct StructuralStats {
        uint32_t chunksAnalyzed;
        uint32_t voxelsUnderStress;
        uint32_t structuralFailures;
        float averageAnalysisTime;
        float totalStabilityScore;
        uint32_t pendingAnalyses;
    };
    StructuralStats getStatistics() const { return stats; }
    
private:
    VoxelWorldManager& worldManager;
    const MaterialPalette& materialPalette;
    
    // Material stress properties lookup
    std::vector<MaterialStressProperties> materialStressProperties;
    
    // Structural data storage
    std::unordered_map<ChunkPos, std::shared_ptr<ChunkStructuralData>, ChunkPosHash> structuralData;
    mutable std::mutex dataMapMutex;
    
    // Analysis queue and threading
    struct AnalysisTask {
        ChunkPos chunkPos;
        float priority;
        float timestamp;
        
        AnalysisTask(const ChunkPos& pos, float pri, float time)
            : chunkPos(pos), priority(pri), timestamp(time) {}
        
        bool operator<(const AnalysisTask& other) const {
            return priority < other.priority;  // Higher priority = lower value
        }
    };
    
    std::priority_queue<AnalysisTask> analysisQueue;
    std::vector<std::thread> analysisThreads;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldStop;
    
    // Configuration
    float stabilityThreshold;
    glm::vec3 gravityVector;
    float analysisFrequency;
    bool enableDetailedAnalysis;
    bool enableFatigueSimulation;
    uint32_t maxAnalysisQueueSize;
    
    // Statistics
    mutable StructuralStats stats;
    mutable std::mutex statsMutex;
    
    // Core analysis algorithms
    void performStructuralAnalysis(const ChunkPos& chunkPos);  // [BACKGROUND_THREAD]
    void calculateSupportValues(ChunkStructuralData& chunkData);  // [BACKGROUND_THREAD]
    void calculateLoadValues(ChunkStructuralData& chunkData);  // [BACKGROUND_THREAD]
    void calculateStressDistribution(ChunkStructuralData& chunkData);  // [BACKGROUND_THREAD]
    
    // Support propagation algorithm
    void propagateSupport(ChunkStructuralData& chunkData);  // [BACKGROUND_THREAD]
    float calculateVoxelSupport(const VoxelPos& pos, const ChunkStructuralData& chunkData);
    bool isVoxelGrounded(const VoxelPos& pos, const ChunkStructuralData& chunkData);
    std::vector<VoxelPos> getNeighborVoxels(const VoxelPos& pos);
    
    // Load calculation algorithm
    void propagateLoad(ChunkStructuralData& chunkData);  // [BACKGROUND_THREAD]
    float calculateVoxelLoad(const VoxelPos& pos, const ChunkStructuralData& chunkData);
    float getVoxelWeight(const VoxelPos& pos);
    
    // Stress analysis (simplified FEA)
    void calculateVonMisesStress(VoxelStressState& stress);
    void calculatePrincipalStresses(VoxelStressState& stress);
    void applyBoundaryConditions(ChunkStructuralData& chunkData);
    
    // Failure detection and propagation
    void detectStructuralFailures(ChunkStructuralData& chunkData);
    void propagateFailure(const VoxelPos& failurePos, ChunkStructuralData& chunkData);
    bool checkFailureCriteria(const VoxelStressState& stress, uint8_t materialID);
    
    // Fatigue simulation
    void updateFatigueAccumulation(ChunkStructuralData& chunkData, float deltaTime);
    float calculateFatigueDamage(const VoxelStressState& stress, uint8_t materialID, float deltaTime);
    
    // Cross-chunk analysis
    void analyzeChunkBoundaries(const ChunkPos& chunkPos);
    void synchronizeNeighborChunks(const ChunkPos& chunkPos);
    std::vector<ChunkPos> getNeighborChunks(const ChunkPos& chunkPos);
    
    // Worker thread functions
    void structuralAnalysisWorker();  // [BACKGROUND_THREAD]
    void processAnalysisQueue();
    
    // Optimization and caching
    void optimizeAnalysisOrder();
    void cacheStableChunks();
    bool isChunkStable(const ChunkStructuralData& chunkData);
    void updateChunkStability(ChunkStructuralData& chunkData);
    
    // Data management
    std::shared_ptr<ChunkStructuralData> getOrCreateStructuralData(const ChunkPos& chunkPos);
    void cleanupUnusedStructuralData();
    void initializeMaterialProperties();
    
    // Helper functions
    int32_t getVoxelIndex(const VoxelPos& localPos);
    VoxelPos getLocalPos(int32_t index);
    float calculateAnalysisPriority(const ChunkPos& chunkPos, float currentTime);
    void updateStatistics(const ChunkStructuralData& chunkData, float analysisTime);
    
    // Debug and visualization support
    void exportStressVisualization(const ChunkPos& chunkPos, const std::string& filename);
    void validateStructuralData(const ChunkStructuralData& chunkData);
};

// ============================================================================
// STRUCTURAL FAILURE PATTERNS
// ============================================================================

/**
 * Predefined structural failure patterns for realistic collapse simulation
 */
class StructuralFailurePatterns {
public:
    StructuralFailurePatterns(StructuralIntegrityManager& structuralMgr);
    
    // Building collapse patterns
    void simulatePancakeCollapse(const glm::vec3& buildingCenter, int32_t floors);
    void simulateProgressiveCollapse(const glm::vec3& triggerPoint, const glm::vec3& buildingBounds);
    void simulateLeaningCollapse(const glm::vec3& base, const glm::vec3& toppleDirection);
    
    // Natural failure patterns
    void simulateRockslide(const glm::vec3& cliffFace, const glm::vec3& slideDirection);
    void simulateLandslide(const glm::vec3& triggerPoint, float slopeAngle);
    void simulateErosionCollapse(const glm::vec3& riverBank, float erosionRate);
    
    // Infrastructure failures
    void simulateBridgeCollapse(const glm::vec3& bridgeStart, const glm::vec3& bridgeEnd);
    void simulateTunnelCollapse(const glm::vec3& tunnelCenter, const glm::vec3& tunnelDirection);
    void simulateDamFailure(const glm::vec3& damBase, float waterPressure);
    
private:
    StructuralIntegrityManager& structuralManager;
    
    // Pattern generation helpers
    void createFailureChain(const std::vector<VoxelPos>& failureSequence, float timeDelay);
    void simulateWeakPointFailure(const VoxelPos& weakPoint, float failureRadius);
    void propagateCollapseWave(const glm::vec3& origin, const glm::vec3& direction, float speed);
}; 