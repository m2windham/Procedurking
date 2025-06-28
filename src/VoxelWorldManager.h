#pragma once
#include "VoxelCore.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <future>
#include <condition_variable>

// ============================================================================
// SPARSE VOXEL OCTREE (SVO) STRUCTURES
// ============================================================================

/**
 * Node in the Sparse Voxel Octree for planetary-scale storage
 */
struct SVONode {
    static constexpr int32_t CHILDREN_COUNT = 8;
    
    // Node data
    uint32_t childMask;        // Bitmask indicating which children exist
    uint8_t materialID;        // Uniform material if leaf node
    uint8_t level;             // Depth in octree (0 = leaf)
    bool isLeaf;               // True if this is a leaf node
    bool isUniform;            // True if all voxels have same material
    
    // Child pointers (only allocated if needed)
    std::unique_ptr<std::array<std::unique_ptr<SVONode>, CHILDREN_COUNT>> children;
    
    SVONode(uint8_t lvl = 0) : childMask(0), materialID(0), level(lvl), isLeaf(true), isUniform(true) {}
    
    bool hasChild(int32_t index) const { return (childMask & (1 << index)) != 0; }
    void setChild(int32_t index) { childMask |= (1 << index); }
    void clearChild(int32_t index) { childMask &= ~(1 << index); }
    
    SVONode* getChild(int32_t index) const {
        if (!hasChild(index) || !children) return nullptr;
        return (*children)[index].get();
    }
    
    void createChild(int32_t index) {
        if (!children) {
            children = std::make_unique<std::array<std::unique_ptr<SVONode>, CHILDREN_COUNT>>();
        }
        (*children)[index] = std::make_unique<SVONode>(level - 1);
        setChild(index);
    }
};

/**
 * Sparse Voxel Octree for efficient planetary storage
 */
class SparseVoxelOctree {
public:
    SparseVoxelOctree(int32_t maxDepth = 20);  // 20 levels = ~1M voxels per side
    ~SparseVoxelOctree();
    
    // Voxel operations
    Voxel getVoxel(const VoxelPos& pos) const;
    void setVoxel(const VoxelPos& pos, const Voxel& voxel);
    bool hasVoxel(const VoxelPos& pos) const;
    
    // Chunk operations
    void loadChunkData(const ChunkPos& chunkPos, WorldChunk& chunk) const;
    void storeChunkData(const ChunkPos& chunkPos, const WorldChunk& chunk);
    
    // Memory management
    void compress();  // Remove redundant nodes
    void optimize();  // Merge uniform regions
    size_t getMemoryUsage() const;
    size_t getNodeCount() const;
    
    // Serialization
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
private:
    std::unique_ptr<SVONode> root;
    int32_t maxDepth;
    int32_t worldSize;  // 2^maxDepth
    
    // Helper functions
    SVONode* findNode(const VoxelPos& pos, bool createPath = false);
    const SVONode* findNode(const VoxelPos& pos) const;
    int32_t getChildIndex(const VoxelPos& pos, int32_t level) const;
    VoxelPos getNodeOrigin(const VoxelPos& pos, int32_t level) const;
    int32_t getNodeSize(int32_t level) const;
    
    // Optimization helpers
    bool canMergeNode(const SVONode* node) const;
    void mergeUniformChildren(SVONode* node);
    void removeEmptyNodes(SVONode* node);
};

// ============================================================================
// CHUNK LOADING AND PAGING SYSTEM
// ============================================================================

/**
 * Asynchronous chunk loading task
 */
struct ChunkLoadTask {
    ChunkPos position;
    std::shared_ptr<WorldChunk> chunk;
    std::promise<bool> promise;
    enum Priority { LOW, NORMAL, HIGH, URGENT } priority;
    
    ChunkLoadTask(const ChunkPos& pos, std::shared_ptr<WorldChunk> c, Priority p = NORMAL)
        : position(pos), chunk(c), priority(p) {}
};

/**
 * Manages the loading, unloading, and paging of world chunks
 * Implements hybrid SVO/hash storage with asynchronous loading
 */
class VoxelWorldManager {
public:
    VoxelWorldManager(const VoxelPlanetData& planetData);
    ~VoxelWorldManager();
    
    // Core voxel operations
    Voxel getVoxel(const VoxelPos& pos);
    void setVoxel(const VoxelPos& pos, const Voxel& voxel);
    bool hasVoxel(const VoxelPos& pos);
    
    // Chunk management
    std::shared_ptr<WorldChunk> getChunk(const ChunkPos& pos);
    void loadChunk(const ChunkPos& pos, ChunkLoadTask::Priority priority = ChunkLoadTask::NORMAL);
    void unloadChunk(const ChunkPos& pos);
    void updateActiveRegion(const glm::vec3& playerPosition, float radius);
    
    // Modification tracking
    void markChunkDirty(const ChunkPos& pos, WorldChunk::State dirtyType);
    std::vector<ChunkPos> getDirtyChunks(WorldChunk::State minDirtyLevel) const;
    void clearDirtyFlag(const ChunkPos& pos);
    
    // Performance management
    void setMaxActiveChunks(int32_t maxChunks) { maxActiveChunks = maxChunks; }
    void setLoadingThreadCount(int32_t threadCount);
    void update(float deltaTime);  // Process loading queue, unload distant chunks
    
    // Memory management
    size_t getMemoryUsage() const;
    void garbageCollect();  // Force unload of unused chunks
    void compressInactiveChunks();
    
    // Serialization
    void saveWorld(const std::string& filename) const;
    void loadWorld(const std::string& filename);
    
    // Statistics
    struct Statistics {
        int32_t activeChunks;
        int32_t loadedChunks;
        int32_t pendingLoads;
        size_t memoryUsage;
        float averageLoadTime;
        float chunkHitRate;
    };
    Statistics getStatistics() const;
    
    // Thread-safe bulk operations
    void setVoxelBulk(const std::vector<std::pair<VoxelPos, Voxel>>& voxels);  // [GPU_CANDIDATE]
    std::vector<Voxel> getVoxelBulk(const std::vector<VoxelPos>& positions);  // [GPU_CANDIDATE]
    
private:
    VoxelPlanetData planetData;
    MaterialPalette materialPalette;
    
    // Storage systems
    std::unique_ptr<SparseVoxelOctree> svoStorage;  // Persistent storage
    std::unordered_map<ChunkPos, std::shared_ptr<WorldChunk>, ChunkPosHash> activeChunks;  // Active memory
    
    // Threading and async loading
    std::vector<std::thread> loadingThreads;
    std::queue<ChunkLoadTask> loadingQueue;
    mutable std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldStop;
    
    // Performance limits
    int32_t maxActiveChunks;
    float unloadDistance;
    float loadDistance;
    
    // Statistics tracking
    mutable std::mutex statsMutex;
    mutable Statistics stats;
    
    // Worker thread functions
    void chunkLoadingWorker();  // [BACKGROUND_THREAD]
    void processLoadingQueue();
    
    // Chunk lifecycle
    std::shared_ptr<WorldChunk> createChunk(const ChunkPos& pos);
    void loadChunkFromSVO(const ChunkPos& pos, WorldChunk& chunk);  // [BACKGROUND_THREAD]
    void storeChunkToSVO(const ChunkPos& pos, const WorldChunk& chunk);  // [BACKGROUND_THREAD]
    
    // Memory management helpers
    void unloadDistantChunks(const glm::vec3& playerPosition);
    std::vector<ChunkPos> getChunksInRadius(const glm::vec3& center, float radius) const;
    float getChunkPriority(const ChunkPos& pos, const glm::vec3& playerPosition) const;
    
    // Coordinate conversion
    ChunkPos worldToChunk(const glm::vec3& worldPos) const;
    VoxelPos worldToVoxel(const glm::vec3& worldPos) const;
    glm::vec3 voxelToWorld(const VoxelPos& voxelPos) const;
    glm::vec3 chunkToWorld(const ChunkPos& chunkPos) const;
}; 