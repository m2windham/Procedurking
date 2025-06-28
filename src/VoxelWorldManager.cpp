#include "VoxelWorldManager.h"
#include <fstream>
#include <algorithm>
#include <iostream>

// ============================================================================
// SPARSE VOXEL OCTREE IMPLEMENTATION
// ============================================================================

SparseVoxelOctree::SparseVoxelOctree(int32_t maxDepth) 
    : maxDepth(maxDepth), worldSize(1 << maxDepth) {
    root = std::make_unique<SVONode>(maxDepth);
}

SparseVoxelOctree::~SparseVoxelOctree() = default;

Voxel SparseVoxelOctree::getVoxel(const VoxelPos& pos) const {
    const SVONode* node = findNode(pos);
    if (!node) {
        return Voxel(MaterialPalette::AIR, 0);
    }
    
    if (node->isLeaf && node->isUniform) {
        return Voxel(node->materialID, 255);
    }
    
    // For non-uniform leaves, we'd need to store voxel data
    // For now, return air
    return Voxel(MaterialPalette::AIR, 0);
}

void SparseVoxelOctree::setVoxel(const VoxelPos& pos, const Voxel& voxel) {
    if (pos.x < 0 || pos.x >= worldSize || 
        pos.y < 0 || pos.y >= worldSize || 
        pos.z < 0 || pos.z >= worldSize) {
        return; // Out of bounds
    }
    
    SVONode* node = findNode(pos, true);
    if (!node) {
        return;
    }
    
    // For now, just set the material at the leaf level
    if (node->isLeaf) {
        node->materialID = voxel.materialID;
        node->isUniform = true;
    }
}

bool SparseVoxelOctree::hasVoxel(const VoxelPos& pos) const {
    const Voxel voxel = getVoxel(pos);
    return voxel.materialID != MaterialPalette::AIR;
}

void SparseVoxelOctree::loadChunkData(const ChunkPos& chunkPos, WorldChunk& chunk) const {
    // Convert chunk position to world voxel coordinates
    VoxelPos chunkOrigin = chunkPos.toVoxelPos(WorldChunk::CHUNK_SIZE);
    
    for (int32_t x = 0; x < WorldChunk::CHUNK_SIZE; ++x) {
        for (int32_t y = 0; y < WorldChunk::CHUNK_SIZE; ++y) {
            for (int32_t z = 0; z < WorldChunk::CHUNK_SIZE; ++z) {
                VoxelPos worldPos(
                    chunkOrigin.x + x,
                    chunkOrigin.y + y,
                    chunkOrigin.z + z
                );
                
                Voxel voxel = getVoxel(worldPos);
                if (voxel.materialID != MaterialPalette::AIR) {
                    chunk.setVoxel(VoxelPos(x, y, z), voxel);
                }
            }
        }
    }
}

void SparseVoxelOctree::storeChunkData(const ChunkPos& chunkPos, const WorldChunk& chunk) {
    // Convert chunk position to world voxel coordinates
    VoxelPos chunkOrigin = chunkPos.toVoxelPos(WorldChunk::CHUNK_SIZE);
    
    for (int32_t x = 0; x < WorldChunk::CHUNK_SIZE; ++x) {
        for (int32_t y = 0; y < WorldChunk::CHUNK_SIZE; ++y) {
            for (int32_t z = 0; z < WorldChunk::CHUNK_SIZE; ++z) {
                VoxelPos localPos(x, y, z);
                VoxelPos worldPos(
                    chunkOrigin.x + x,
                    chunkOrigin.y + y,
                    chunkOrigin.z + z
                );
                
                const Voxel& voxel = chunk.getVoxel(localPos);
                setVoxel(worldPos, voxel);
            }
        }
    }
}

void SparseVoxelOctree::compress() {
    // Merge uniform child nodes
    if (root) {
        mergeUniformChildren(root.get());
    }
}

void SparseVoxelOctree::optimize() {
    compress();
    // Could add more optimizations here
}

size_t SparseVoxelOctree::getMemoryUsage() const {
    // Simplified calculation
    return getNodeCount() * sizeof(SVONode);
}

size_t SparseVoxelOctree::getNodeCount() const {
    // Simplified - would need recursive traversal for accurate count
    return 1000; // Placeholder
}

void SparseVoxelOctree::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    // Write header
    file.write(reinterpret_cast<const char*>(&maxDepth), sizeof(maxDepth));
    file.write(reinterpret_cast<const char*>(&worldSize), sizeof(worldSize));
    
    // TODO: Implement recursive node serialization
    file.close();
}

void SparseVoxelOctree::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }
    
    // Read header
    file.read(reinterpret_cast<char*>(&maxDepth), sizeof(maxDepth));
    file.read(reinterpret_cast<char*>(&worldSize), sizeof(worldSize));
    
    // TODO: Implement recursive node deserialization
    file.close();
}

SVONode* SparseVoxelOctree::findNode(const VoxelPos& pos, bool createPath) {
    if (!root) {
        return nullptr;
    }
    
    SVONode* current = root.get();
    int32_t currentSize = worldSize;
    
    for (int32_t level = maxDepth; level > 0; --level) {
        if (current->isLeaf) {
            if (createPath && level > 1) {
                // Convert leaf to internal node
                current->isLeaf = false;
                current->children = std::make_unique<std::array<std::unique_ptr<SVONode>, SVONode::CHILDREN_COUNT>>();
            } else {
                return current;
            }
        }
        
        currentSize /= 2;
        int32_t childIndex = getChildIndex(pos, level);
        
        if (!current->hasChild(childIndex)) {
            if (createPath) {
                current->createChild(childIndex);
            } else {
                return nullptr;
            }
        }
        
        current = current->getChild(childIndex);
        if (!current) {
            return nullptr;
        }
    }
    
    return current;
}

const SVONode* SparseVoxelOctree::findNode(const VoxelPos& pos) const {
    return const_cast<SparseVoxelOctree*>(this)->findNode(pos, false);
}

int32_t SparseVoxelOctree::getChildIndex(const VoxelPos& pos, int32_t level) const {
    int32_t nodeSize = getNodeSize(level - 1);
    int32_t index = 0;
    
    if (pos.x >= nodeSize) index |= 1;
    if (pos.y >= nodeSize) index |= 2;
    if (pos.z >= nodeSize) index |= 4;
    
    return index;
}

VoxelPos SparseVoxelOctree::getNodeOrigin(const VoxelPos& pos, int32_t level) const {
    int32_t nodeSize = getNodeSize(level);
    return VoxelPos(
        (pos.x / nodeSize) * nodeSize,
        (pos.y / nodeSize) * nodeSize,
        (pos.z / nodeSize) * nodeSize
    );
}

int32_t SparseVoxelOctree::getNodeSize(int32_t level) const {
    return 1 << level;
}

bool SparseVoxelOctree::canMergeNode(const SVONode* node) const {
    if (!node || node->isLeaf || !node->children) {
        return false;
    }
    
    // Check if all children are uniform leaves with the same material
    uint8_t firstMaterial = 255;
    bool firstSet = false;
    
    for (int32_t i = 0; i < SVONode::CHILDREN_COUNT; ++i) {
        if (node->hasChild(i)) {
            const SVONode* child = node->getChild(i);
            if (!child || !child->isLeaf || !child->isUniform) {
                return false;
            }
            
            if (!firstSet) {
                firstMaterial = child->materialID;
                firstSet = true;
            } else if (child->materialID != firstMaterial) {
                return false;
            }
        }
    }
    
    return firstSet;
}

void SparseVoxelOctree::mergeUniformChildren(SVONode* node) {
    if (!node || node->isLeaf) {
        return;
    }
    
    // Recursively process children first
    if (node->children) {
        for (int32_t i = 0; i < SVONode::CHILDREN_COUNT; ++i) {
            if (node->hasChild(i)) {
                SVONode* child = node->getChild(i);
                if (child) {
                    mergeUniformChildren(child);
                }
            }
        }
    }
    
    // Check if this node can be merged
    if (canMergeNode(node)) {
        // Get the uniform material from first child
        uint8_t uniformMaterial = MaterialPalette::AIR;
        for (int32_t i = 0; i < SVONode::CHILDREN_COUNT; ++i) {
            if (node->hasChild(i)) {
                const SVONode* child = node->getChild(i);
                if (child) {
                    uniformMaterial = child->materialID;
                    break;
                }
            }
        }
        
        // Convert to uniform leaf
        node->children.reset();
        node->childMask = 0;
        node->isLeaf = true;
        node->isUniform = true;
        node->materialID = uniformMaterial;
    }
}

void SparseVoxelOctree::removeEmptyNodes(SVONode* node) {
    if (!node || node->isLeaf) {
        return;
    }
    
    // Recursively process children
    if (node->children) {
        for (int32_t i = 0; i < SVONode::CHILDREN_COUNT; ++i) {
            if (node->hasChild(i)) {
                SVONode* child = node->getChild(i);
                if (child) {
                    removeEmptyNodes(child);
                    
                    // Remove child if it's an empty leaf
                    if (child->isLeaf && child->isUniform && child->materialID == MaterialPalette::AIR) {
                        (*node->children)[i].reset();
                        node->clearChild(i);
                    }
                }
            }
        }
        
        // If no children remain, convert to leaf
        if (node->childMask == 0) {
            node->children.reset();
            node->isLeaf = true;
            node->isUniform = true;
            node->materialID = MaterialPalette::AIR;
        }
    }
}

// ============================================================================
// VOXEL WORLD MANAGER IMPLEMENTATION
// ============================================================================

VoxelWorldManager::VoxelWorldManager(const VoxelPlanetData& planetData)
    : planetData(planetData), maxActiveChunks(1000), unloadDistance(500.0f), loadDistance(300.0f), shouldStop(false) {
    
    svoStorage = std::make_unique<SparseVoxelOctree>(20); // 20 levels = ~1M voxels per side
    
    // Initialize statistics
    stats.activeChunks = 0;
    stats.loadedChunks = 0;
    stats.pendingLoads = 0;
    stats.memoryUsage = 0;
    stats.averageLoadTime = 0.0f;
    stats.chunkHitRate = 1.0f;
    
    // Start loading threads
    setLoadingThreadCount(4);
}

VoxelWorldManager::~VoxelWorldManager() {
    shouldStop = true;
    queueCondition.notify_all();
    
    for (auto& thread : loadingThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

Voxel VoxelWorldManager::getVoxel(const VoxelPos& pos) {
    ChunkPos chunkPos(pos, WorldChunk::CHUNK_SIZE);
    auto chunk = getChunk(chunkPos);
    
    if (!chunk) {
        return Voxel(MaterialPalette::AIR, 0);
    }
    
    // Convert world position to local chunk position
    VoxelPos localPos(
        pos.x % WorldChunk::CHUNK_SIZE,
        pos.y % WorldChunk::CHUNK_SIZE,
        pos.z % WorldChunk::CHUNK_SIZE
    );
    
    return chunk->getVoxel(localPos);
}

void VoxelWorldManager::setVoxel(const VoxelPos& pos, const Voxel& voxel) {
    ChunkPos chunkPos(pos, WorldChunk::CHUNK_SIZE);
    auto chunk = getChunk(chunkPos);
    
    if (!chunk) {
        // Load chunk if needed
        loadChunk(chunkPos, ChunkLoadTask::URGENT);
        chunk = getChunk(chunkPos);
        if (!chunk) {
            return;
        }
    }
    
    // Convert world position to local chunk position
    VoxelPos localPos(
        pos.x % WorldChunk::CHUNK_SIZE,
        pos.y % WorldChunk::CHUNK_SIZE,
        pos.z % WorldChunk::CHUNK_SIZE
    );
    
    chunk->setVoxel(localPos, voxel);
    markChunkDirty(chunkPos, WorldChunk::State::DIRTY_MESH);
}

bool VoxelWorldManager::hasVoxel(const VoxelPos& pos) {
    const Voxel voxel = getVoxel(pos);
    return voxel.materialID != MaterialPalette::AIR;
}

std::shared_ptr<WorldChunk> VoxelWorldManager::getChunk(const ChunkPos& pos) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    auto it = activeChunks.find(pos);
    if (it != activeChunks.end()) {
        return it->second;
    }
    
    return nullptr;
}

void VoxelWorldManager::loadChunk(const ChunkPos& pos, ChunkLoadTask::Priority priority) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        
        // Check if already loaded or loading
        if (activeChunks.find(pos) != activeChunks.end()) {
            return;
        }
    }
    
    auto chunk = createChunk(pos);
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        activeChunks[pos] = chunk;
        
        // Add to loading queue
        ChunkLoadTask task(pos, chunk, priority);
        loadingQueue.push(std::move(task));
        stats.pendingLoads++;
    }
    
    queueCondition.notify_one();
}

void VoxelWorldManager::unloadChunk(const ChunkPos& pos) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    auto it = activeChunks.find(pos);
    if (it != activeChunks.end()) {
        // Store chunk data to SVO before unloading
        storeChunkToSVO(pos, *it->second);
        activeChunks.erase(it);
        stats.activeChunks--;
    }
}

void VoxelWorldManager::updateActiveRegion(const glm::vec3& playerPosition, float radius) {
    ChunkPos playerChunk = worldToChunk(playerPosition);
    
    // Load chunks around player
    int32_t chunkRadius = static_cast<int32_t>(radius / WorldChunk::CHUNK_SIZE) + 1;
    
    for (int32_t x = -chunkRadius; x <= chunkRadius; ++x) {
        for (int32_t y = -chunkRadius; y <= chunkRadius; ++y) {
            for (int32_t z = -chunkRadius; z <= chunkRadius; ++z) {
                ChunkPos chunkPos(
                    playerChunk.x + x,
                    playerChunk.y + y,
                    playerChunk.z + z
                );
                
                glm::vec3 chunkCenter = chunkToWorld(chunkPos);
                float distance = glm::length(chunkCenter - playerPosition);
                
                if (distance <= loadDistance) {
                    loadChunk(chunkPos, ChunkLoadTask::NORMAL);
                }
            }
        }
    }
    
    // Unload distant chunks
    unloadDistantChunks(playerPosition);
}

void VoxelWorldManager::markChunkDirty(const ChunkPos& pos, WorldChunk::State dirtyType) {
    auto chunk = getChunk(pos);
    if (chunk && chunk->getState() < dirtyType) {
        chunk->setState(dirtyType);
    }
}

std::vector<ChunkPos> VoxelWorldManager::getDirtyChunks(WorldChunk::State minDirtyLevel) const {
    std::vector<ChunkPos> dirtyChunks;
    
    std::lock_guard<std::mutex> lock(queueMutex);
    for (const auto& [pos, chunk] : activeChunks) {
        if (chunk->getState() >= minDirtyLevel) {
            dirtyChunks.push_back(pos);
        }
    }
    
    return dirtyChunks;
}

void VoxelWorldManager::clearDirtyFlag(const ChunkPos& pos) {
    auto chunk = getChunk(pos);
    if (chunk) {
        chunk->setState(WorldChunk::State::ACTIVE);
    }
}

void VoxelWorldManager::setLoadingThreadCount(int32_t threadCount) {
    // Stop existing threads
    shouldStop = true;
    queueCondition.notify_all();
    
    for (auto& thread : loadingThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    loadingThreads.clear();
    shouldStop = false;
    
    // Start new threads
    for (int32_t i = 0; i < threadCount; ++i) {
        loadingThreads.emplace_back(&VoxelWorldManager::chunkLoadingWorker, this);
    }
}

void VoxelWorldManager::update(float deltaTime) {
    (void)deltaTime; // Suppress unused parameter warning
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(statsMutex);
        stats.activeChunks = static_cast<int32_t>(activeChunks.size());
        stats.loadedChunks = stats.activeChunks;
        
        size_t totalMemory = 0;
        for (const auto& [pos, chunk] : activeChunks) {
            totalMemory += chunk->getMemoryUsage();
        }
        stats.memoryUsage = totalMemory;
    }
}

size_t VoxelWorldManager::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return stats.memoryUsage;
}

void VoxelWorldManager::garbageCollect() {
    // Force unload of inactive chunks
    std::vector<ChunkPos> chunksToUnload;
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (const auto& [pos, chunk] : activeChunks) {
            if (chunk->getState() == WorldChunk::State::ACTIVE) {
                chunksToUnload.push_back(pos);
            }
        }
    }
    
    // Unload half of the inactive chunks
    size_t unloadCount = chunksToUnload.size() / 2;
    for (size_t i = 0; i < unloadCount; ++i) {
        unloadChunk(chunksToUnload[i]);
    }
}

void VoxelWorldManager::compressInactiveChunks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    for (const auto& [pos, chunk] : activeChunks) {
        if (chunk->getState() == WorldChunk::State::ACTIVE) {
            chunk->compress();
        }
    }
}

VoxelWorldManager::Statistics VoxelWorldManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return stats;
}

void VoxelWorldManager::chunkLoadingWorker() {
    while (!shouldStop) {
        std::unique_lock<std::mutex> lock(queueMutex);
        
        queueCondition.wait(lock, [this] {
            return !loadingQueue.empty() || shouldStop;
        });
        
        if (shouldStop) {
            break;
        }
        
        if (loadingQueue.empty()) {
            continue;
        }
        
        ChunkLoadTask task = std::move(loadingQueue.front());
        loadingQueue.pop();
        stats.pendingLoads--;
        
        lock.unlock();
        
        // Load chunk data from SVO
        loadChunkFromSVO(task.position, *task.chunk);
        
        // Mark chunk as active
        task.chunk->setState(WorldChunk::State::ACTIVE);
        
        // Fulfill promise
        task.promise.set_value(true);
    }
}

std::shared_ptr<WorldChunk> VoxelWorldManager::createChunk(const ChunkPos& pos) {
    return std::make_shared<WorldChunk>(pos);
}

void VoxelWorldManager::loadChunkFromSVO(const ChunkPos& pos, WorldChunk& chunk) {
    if (svoStorage) {
        svoStorage->loadChunkData(pos, chunk);
    }
}

void VoxelWorldManager::storeChunkToSVO(const ChunkPos& pos, const WorldChunk& chunk) {
    if (svoStorage) {
        svoStorage->storeChunkData(pos, chunk);
    }
}

void VoxelWorldManager::unloadDistantChunks(const glm::vec3& playerPosition) {
    std::vector<ChunkPos> chunksToUnload;
    
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (const auto& [pos, chunk] : activeChunks) {
            glm::vec3 chunkCenter = chunkToWorld(pos);
            float distance = glm::length(chunkCenter - playerPosition);
            
            if (distance > unloadDistance) {
                chunksToUnload.push_back(pos);
            }
        }
    }
    
    for (const auto& pos : chunksToUnload) {
        unloadChunk(pos);
    }
}

ChunkPos VoxelWorldManager::worldToChunk(const glm::vec3& worldPos) const {
    return ChunkPos(
        static_cast<int32_t>(worldPos.x) / WorldChunk::CHUNK_SIZE,
        static_cast<int32_t>(worldPos.y) / WorldChunk::CHUNK_SIZE,
        static_cast<int32_t>(worldPos.z) / WorldChunk::CHUNK_SIZE
    );
}

VoxelPos VoxelWorldManager::worldToVoxel(const glm::vec3& worldPos) const {
    return VoxelPos(
        static_cast<int32_t>(worldPos.x / planetData.voxelSize),
        static_cast<int32_t>(worldPos.y / planetData.voxelSize),
        static_cast<int32_t>(worldPos.z / planetData.voxelSize)
    );
}

glm::vec3 VoxelWorldManager::voxelToWorld(const VoxelPos& voxelPos) const {
    return glm::vec3(
        static_cast<float>(voxelPos.x) * planetData.voxelSize,
        static_cast<float>(voxelPos.y) * planetData.voxelSize,
        static_cast<float>(voxelPos.z) * planetData.voxelSize
    );
}

glm::vec3 VoxelWorldManager::chunkToWorld(const ChunkPos& chunkPos) const {
    return glm::vec3(
        static_cast<float>(chunkPos.x * WorldChunk::CHUNK_SIZE) * planetData.voxelSize,
        static_cast<float>(chunkPos.y * WorldChunk::CHUNK_SIZE) * planetData.voxelSize,
        static_cast<float>(chunkPos.z * WorldChunk::CHUNK_SIZE) * planetData.voxelSize
    );
}

void VoxelWorldManager::saveWorld(const std::string& filename) const {
    if (svoStorage) {
        svoStorage->saveToFile(filename);
    }
}

void VoxelWorldManager::loadWorld(const std::string& filename) {
    if (svoStorage) {
        svoStorage->loadFromFile(filename);
    }
}

void VoxelWorldManager::setVoxelBulk(const std::vector<std::pair<VoxelPos, Voxel>>& voxels) {
    for (const auto& [pos, voxel] : voxels) {
        setVoxel(pos, voxel);
    }
}

std::vector<Voxel> VoxelWorldManager::getVoxelBulk(const std::vector<VoxelPos>& positions) {
    std::vector<Voxel> voxels;
    voxels.reserve(positions.size());
    
    for (const auto& pos : positions) {
        voxels.push_back(getVoxel(pos));
    }
    
    return voxels;
}

std::vector<ChunkPos> VoxelWorldManager::getChunksInRadius(const glm::vec3& center, float radius) const {
    std::vector<ChunkPos> chunks;
    ChunkPos centerChunk = worldToChunk(center);
    
    int32_t chunkRadius = static_cast<int32_t>(radius / WorldChunk::CHUNK_SIZE) + 1;
    
    for (int32_t x = -chunkRadius; x <= chunkRadius; ++x) {
        for (int32_t y = -chunkRadius; y <= chunkRadius; ++y) {
            for (int32_t z = -chunkRadius; z <= chunkRadius; ++z) {
                ChunkPos chunkPos(
                    centerChunk.x + x,
                    centerChunk.y + y,
                    centerChunk.z + z
                );
                
                glm::vec3 chunkCenter = chunkToWorld(chunkPos);
                float distance = glm::length(chunkCenter - center);
                
                if (distance <= radius) {
                    chunks.push_back(chunkPos);
                }
            }
        }
    }
    
    return chunks;
}

float VoxelWorldManager::getChunkPriority(const ChunkPos& pos, const glm::vec3& playerPosition) const {
    glm::vec3 chunkCenter = chunkToWorld(pos);
    float distance = glm::length(chunkCenter - playerPosition);
    
    // Lower distance = higher priority (lower value)
    return distance;
}

void VoxelWorldManager::processLoadingQueue() {
    // This function is called by the worker threads
    // Implementation is handled in chunkLoadingWorker()
} 