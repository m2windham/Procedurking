#include "DestructionManager.h"
#include <algorithm>
#include <memory>

DestructionManager::DestructionManager(VoxelWorldManager& worldManager, const MaterialPalette& palette)
    : worldManager(worldManager), materialPalette(palette), enableChainReactions(false), chainReactionThreshold(0.5f), maxConcurrentDestructions(2), maxDebrisObjects(128), debrisLifetime(10.0f), nextDebrisID(1), shouldStop(false) {}
DestructionManager::~DestructionManager() {}

void DestructionManager::applyDestruction(const ImpactParameters& impact) {
    // Minimal: just call explosion
    applyExplosion(impact.epicenter, impact.radius, impact.maxDamage);
}

void DestructionManager::applyExplosion(const glm::vec3& center, float radius, float damage) {
    // Find voxels in radius and set to air
    std::vector<VoxelPos> affected;
    for (int x = -int(radius); x <= int(radius); ++x)
        for (int y = -int(radius); y <= int(radius); ++y)
            for (int z = -int(radius); z <= int(radius); ++z) {
                glm::vec3 pos = center + glm::vec3(x, y, z);
                if (glm::length(glm::vec3(x, y, z)) <= radius) {
                    VoxelPos vpos(int(pos.x), int(pos.y), int(pos.z));
                    if (worldManager.hasVoxel(vpos)) {
                        worldManager.setVoxel(vpos, Voxel(0, 0));
                        affected.push_back(vpos);
                    }
                }
            }
    // Create debris object (placeholder)
    if (!affected.empty()) {
        auto debris = std::make_shared<DebrisObject>();
        debris->id = nextDebrisID++;
        debris->voxelPositions = affected;
        debris->centerOfMass = center;
        debris->mass = float(affected.size());
        debris->isStatic = false;
        activeDebris[debris->id] = debris;
    }
}

void DestructionManager::applyProjectileImpact(const glm::vec3& start, const glm::vec3& end, float radius, float damage) {
    // Minimal: treat as explosion at end
    applyExplosion(end, radius, damage);
}

void DestructionManager::applyFireDamage(const glm::vec3& center, float radius, float deltaTime) {}
void DestructionManager::applyAcidCorrosion(const glm::vec3& center, float radius, float deltaTime) {}
void DestructionManager::applySeismicDamage(const glm::vec3& epicenter, float magnitude, float deltaTime) {}

void DestructionManager::update(float deltaTime) {
    // Remove expired debris (placeholder: none expire yet)
}

std::vector<std::shared_ptr<DebrisObject>> DestructionManager::getDebrisInRadius(const glm::vec3& center, float radius) {
    std::vector<std::shared_ptr<DebrisObject>> result;
    for (const auto& [id, debris] : activeDebris) {
        if (glm::length(debris->centerOfMass - center) <= radius)
            result.push_back(debris);
    }
    return result;
}

void DestructionManager::removeDebris(uint32_t debrisID) {
    activeDebris.erase(debrisID);
}
void DestructionManager::clearAllDebris() {
    activeDebris.clear();
}
void DestructionManager::enableChainReactions(bool enable) { enableChainReactions = enable; }
void DestructionManager::setChainReactionThreshold(float threshold) { chainReactionThreshold = threshold; }
void DestructionManager::setMaxConcurrentDestructions(int32_t maxDestructions) { maxConcurrentDestructions = maxDestructions; }
void DestructionManager::setMaxDebrisObjects(uint32_t maxDebris) { maxDebrisObjects = maxDebris; }
void DestructionManager::setDebrisLifetime(float lifetime) { debrisLifetime = lifetime; }
DestructionManager::DestructionStats DestructionManager::getStatistics() const { return stats; } 