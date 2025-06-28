#pragma once
#include "VoxelCore.h"
#include <glad/glad.h>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <future>
#include <condition_variable>
#include <atomic>

// ============================================================================
// MESH DATA STRUCTURES
// ============================================================================

/**
 * Vertex data for voxel meshes
 */
struct VoxelVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
    uint8_t materialID;
    uint8_t ambientOcclusion;  // 0-255 for AO strength
    
    VoxelVertex() = default;
    VoxelVertex(const glm::vec3& pos, const glm::vec3& norm, uint8_t matID)
        : position(pos), normal(norm), texCoords(0.0f), color(1.0f), 
          materialID(matID), ambientOcclusion(255) {}
};

/**
 * Mesh data for a single chunk
 */
struct ChunkMesh {
    std::vector<VoxelVertex> vertices;
    std::vector<uint32_t> indices;
    
    // OpenGL resources
    GLuint VAO, VBO, EBO;
    bool isUploaded;
    
    // Mesh properties
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    size_t triangleCount;
    
    ChunkMesh() : VAO(0), VBO(0), EBO(0), isUploaded(false), triangleCount(0) {}
    ~ChunkMesh();
    
    void uploadToGPU();
    void releaseGPUResources();
    size_t getMemoryUsage() const;
    void clear();
};

/**
 * Level of Detail (LOD) mesh data
 */
struct LODMesh {
    std::array<std::unique_ptr<ChunkMesh>, 4> lodLevels;  // LOD 0-3
    float lodDistances[4] = {64.0f, 128.0f, 256.0f, 512.0f};
    
    ChunkMesh* getMeshForDistance(float distance) const;
    void generateLODLevels(const ChunkMesh& fullMesh);
    void clear();
};

// ============================================================================
// DUAL CONTOURING IMPLEMENTATION
// ============================================================================

/**
 * Edge intersection data for Dual Contouring
 */
struct EdgeIntersection {
    glm::vec3 position;
    glm::vec3 normal;
    uint8_t materialA, materialB;
    bool isValid;
    
    EdgeIntersection() : position(0.0f), normal(0.0f), materialA(0), materialB(0), isValid(false) {}
};

/**
 * Cell data for Dual Contouring algorithm
 */
struct DCCell {
    std::array<float, 8> cornerValues;  // Density values at cell corners
    std::array<uint8_t, 8> cornerMaterials;  // Material IDs at corners
    std::array<EdgeIntersection, 12> edgeIntersections;  // 12 edges per cube
    
    glm::vec3 vertexPosition;  // Generated vertex position
    glm::vec3 vertexNormal;    // Generated vertex normal
    uint8_t dominantMaterial;  // Most common material in cell
    bool hasIntersection;      // True if cell contains surface
    
    DCCell() : vertexPosition(0.0f), vertexNormal(0.0f), dominantMaterial(0), hasIntersection(false) {}
};

/**
 * GPU-accelerated Dual Contouring mesher
 */
class DualContouringMesher {
public:
    DualContouringMesher();
    ~DualContouringMesher();
    
    // Mesh generation
    std::unique_ptr<ChunkMesh> generateMesh(const WorldChunk& chunk, 
                                           const VoxelPlanetData& planetData);  // [GPU_CANDIDATE]
    
    // Batch processing for multiple chunks
    void generateMeshesBatch(const std::vector<std::shared_ptr<WorldChunk>>& chunks,
                            std::vector<std::unique_ptr<ChunkMesh>>& meshes,
                            const VoxelPlanetData& planetData);  // [GPU_CANDIDATE]
    
    // LOD mesh generation
    std::unique_ptr<LODMesh> generateLODMesh(const WorldChunk& chunk,
                                            const VoxelPlanetData& planetData);  // [GPU_CANDIDATE]
    
    // Configuration
    void setIsoValue(float value) { isoValue = value; }
    void setEnableAmbientOcclusion(bool enable) { enableAO = enable; }
    void setEnableGPUAcceleration(bool enable) { useGPU = enable; }
    void setMaxVerticesPerChunk(uint32_t maxVerts) { maxVerticesPerChunk = maxVerts; }
    
    // Performance monitoring
    struct MeshingStats {
        float averageMeshTime;
        uint32_t chunksProcessed;
        uint32_t verticesGenerated;
        uint32_t trianglesGenerated;
        float gpuUtilization;
    };
    MeshingStats getStatistics() const { return stats; }
    
private:
    float isoValue;
    bool enableAO;
    bool useGPU;
    uint32_t maxVerticesPerChunk;
    
    // OpenGL compute shader resources
    GLuint computeProgram;
    GLuint densitySSBO;
    GLuint materialSSBO;
    GLuint vertexSSBO;
    GLuint indexSSBO;
    GLuint cellSSBO;
    
    // CPU fallback data
    std::vector<DCCell> cellGrid;
    std::vector<VoxelVertex> tempVertices;
    std::vector<uint32_t> tempIndices;
    
    // Statistics
    mutable MeshingStats stats;
    mutable std::mutex statsMutex;
    
    // GPU implementation
    void initializeComputeShaders();
    void generateMeshGPU(const WorldChunk& chunk, ChunkMesh& mesh, 
                        const VoxelPlanetData& planetData);  // [GPU_COMPUTE_SHADER]
    void uploadChunkData(const WorldChunk& chunk);
    void downloadMeshData(ChunkMesh& mesh);
    
    // CPU implementation
    void generateMeshCPU(const WorldChunk& chunk, ChunkMesh& mesh,
                        const VoxelPlanetData& planetData);
    void extractIsosurface(const WorldChunk& chunk, const VoxelPlanetData& planetData);
    void generateVertices();
    void generateIndices();
    
    // Dual Contouring core algorithms
    void findEdgeIntersections(const WorldChunk& chunk, const VoxelPlanetData& planetData);
    glm::vec3 calculateVertexPosition(const DCCell& cell);
    glm::vec3 calculateVertexNormal(const DCCell& cell);
    uint8_t getDominantMaterial(const DCCell& cell);
    
    // Ambient occlusion calculation
    uint8_t calculateAmbientOcclusion(const glm::vec3& position, const glm::vec3& normal,
                                     const WorldChunk& chunk);  // [GPU_CANDIDATE]
    
    // Mesh optimization
    void optimizeMesh(ChunkMesh& mesh);
    void removeDuplicateVertices(ChunkMesh& mesh);
    void generateSmoothNormals(ChunkMesh& mesh);
    void calculateBoundingBox(ChunkMesh& mesh);
    
    // LOD generation
    void generateLODLevel(const ChunkMesh& fullMesh, ChunkMesh& lodMesh, int32_t lodLevel);
    void decimateMesh(const ChunkMesh& input, ChunkMesh& output, float reductionRatio);
    
    // Helper functions
    int32_t getCellIndex(int32_t x, int32_t y, int32_t z) const;
    glm::vec3 getWorldPosition(const VoxelPos& voxelPos, const VoxelPlanetData& planetData) const;
    float getDensityValue(const WorldChunk& chunk, const VoxelPos& pos) const;
    
    // Compute shader management
    void updateComputeShaderUniforms(const VoxelPlanetData& planetData);
    void dispatchComputeShader(uint32_t numCells);
    void synchronizeGPU();
};

// ============================================================================
// MESH MANAGEMENT SYSTEM
// ============================================================================

/**
 * Manages mesh generation, caching, and LOD for the entire world
 */
class VoxelMeshManager {
public:
    VoxelMeshManager(const MaterialPalette& palette);
    ~VoxelMeshManager();
    
    // Mesh generation requests
    void requestMesh(const ChunkPos& chunkPos, std::shared_ptr<WorldChunk> chunk,
                    const VoxelPlanetData& planetData);
    void requestLODMesh(const ChunkPos& chunkPos, std::shared_ptr<WorldChunk> chunk,
                       const VoxelPlanetData& planetData);
    
    // Mesh retrieval
    std::shared_ptr<ChunkMesh> getMesh(const ChunkPos& chunkPos);
    std::shared_ptr<LODMesh> getLODMesh(const ChunkPos& chunkPos);
    
    // Mesh invalidation
    void invalidateMesh(const ChunkPos& chunkPos);
    void invalidateRegion(const glm::vec3& center, float radius);
    
    // Background processing
    void update(float deltaTime);  // Process generation queue
    void setMaxConcurrentGenerations(int32_t maxConcurrent);
    
    // Memory management
    void setMaxCachedMeshes(uint32_t maxMeshes);
    void garbageCollect();
    size_t getMemoryUsage() const;
    
    // Rendering support
    std::vector<std::shared_ptr<ChunkMesh>> getMeshesInFrustum(const glm::mat4& viewProjection);
    std::vector<std::shared_ptr<ChunkMesh>> getMeshesInRadius(const glm::vec3& center, float radius);
    
    // Statistics
    struct ManagerStats {
        uint32_t activeMeshes;
        uint32_t cachedMeshes;
        uint32_t pendingGenerations;
        float averageGenerationTime;
        size_t totalMemoryUsage;
    };
    ManagerStats getStatistics() const;
    
private:
    const MaterialPalette& materialPalette;
    std::unique_ptr<DualContouringMesher> mesher;
    
    // Mesh storage
    std::unordered_map<ChunkPos, std::shared_ptr<ChunkMesh>, ChunkPosHash> meshCache;
    std::unordered_map<ChunkPos, std::shared_ptr<LODMesh>, ChunkPosHash> lodMeshCache;
    
    // Generation queue and threading
    struct MeshGenerationTask {
        ChunkPos position;
        std::shared_ptr<WorldChunk> chunk;
        VoxelPlanetData planetData;
        bool generateLOD;
        std::promise<std::shared_ptr<ChunkMesh>> promise;
    };
    
    std::queue<MeshGenerationTask> generationQueue;
    std::vector<std::thread> generationThreads;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldStop;
    
    // Performance limits
    uint32_t maxCachedMeshes;
    int32_t maxConcurrentGenerations;
    
    // Statistics
    mutable ManagerStats managerStats;
    mutable std::mutex statsMutex;
    
    // Worker functions
    void meshGenerationWorker();  // [BACKGROUND_THREAD]
    void processMeshGeneration(MeshGenerationTask& task);
    
    // Cache management
    void evictOldMeshes();
    void cleanupGPUResources();
    float getMeshPriority(const ChunkPos& pos) const;
}; 