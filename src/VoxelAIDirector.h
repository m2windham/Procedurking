#pragma once
#include "VoxelCore.h"
#include "VoxelWorldManager.h"
#include "VoxelPlanetGenerator.h"
#include "DestructionManager.h"
#include "VoxelClimateSimulator.h"
#include <queue>
#include <map>
#include <deque>
#include <mutex>
#include <functional>

// ============================================================================
// PLAYER ACTION TRACKING
// ============================================================================

/**
 * Types of player actions that influence world generation
 */
enum class PlayerActionType {
    // Destruction actions
    EXPLOSION_SMALL,
    EXPLOSION_MEDIUM,
    EXPLOSION_LARGE,
    PROJECTILE_IMPACT,
    STRUCTURAL_DEMOLITION,
    TERRAIN_EXCAVATION,
    
    // Construction actions
    TERRAIN_BUILDING,
    STRUCTURE_CREATION,
    MATERIAL_PLACEMENT,
    FOUNDATION_LAYING,
    
    // Environmental manipulation
    FIRE_IGNITION,
    WATER_REDIRECTION,
    CLIMATE_MODIFICATION,
    WEATHER_CONTROL,
    
    // Geological intervention
    EARTHQUAKE_TRIGGER,
    VOLCANIC_ACTIVATION,
    METEOR_SUMMONING,
    TECTONIC_MANIPULATION,
    
    // Biological influence
    ECOSYSTEM_MODIFICATION,
    SPECIES_INTRODUCTION,
    EVOLUTION_ACCELERATION,
    HABITAT_CREATION,
    
    // Resource management
    MINING_OPERATION,
    RESOURCE_EXTRACTION,
    MATERIAL_SYNTHESIS,
    ENERGY_HARVESTING
};

/**
 * Individual player action record
 */
struct PlayerAction {
    PlayerActionType type;
    glm::vec3 location;
    float magnitude;
    float timestamp;
    glm::vec3 affectedArea;  // Bounding box of affected region
    
    // Action context
    std::vector<uint8_t> materialsInvolved;
    float environmentalImpact;
    bool causedChainReaction;
    uint32_t voxelsAffected;
    
    PlayerAction(PlayerActionType t, const glm::vec3& loc, float mag, float time)
        : type(t), location(loc), magnitude(mag), timestamp(time),
          affectedArea(0.0f), environmentalImpact(0.0f), causedChainReaction(false), voxelsAffected(0) {}
};

/**
 * Player behavior analysis and pattern recognition
 */
class PlayerBehaviorAnalyzer {
public:
    PlayerBehaviorAnalyzer();
    
    void recordAction(const PlayerAction& action);
    void update(float deltaTime);
    
    // Behavior pattern queries
    float getDestructionPreference() const { return destructionPreference; }
    float getConstructionPreference() const { return constructionPreference; }
    float getEnvironmentalFocus() const { return environmentalFocus; }
    float getScalePreference() const { return scalePreference; }  // Small vs large scale actions
    
    // Preferred materials and environments
    std::vector<uint8_t> getPreferredMaterials() const;
    std::vector<PlayerActionType> getFrequentActions() const;
    glm::vec3 getPrimaryActivityCenter() const;
    float getActivityRadius() const;
    
    // Temporal patterns
    float getActionFrequency() const { return actionFrequency; }
    float getSessionDuration() const { return sessionDuration; }
    bool isPlayerActive() const { return isActive; }
    
    // Challenge adaptation
    float getDifficultyPreference() const;
    bool prefersComplexChallenges() const;
    bool prefersCreativeMode() const;
    
private:
    std::deque<PlayerAction> recentActions;
    std::map<PlayerActionType, uint32_t> actionCounts;
    std::map<uint8_t, uint32_t> materialUsage;
    
    // Behavior metrics
    float destructionPreference;
    float constructionPreference;
    float environmentalFocus;
    float scalePreference;
    float actionFrequency;
    float sessionDuration;
    bool isActive;
    
    // Activity tracking
    glm::vec3 activityCenter;
    float activityRadius;
    float lastActionTime;
    
    // Analysis functions
    void updateBehaviorMetrics();
    void updateActivityCenter();
    void updatePreferences();
    void cleanupOldActions(float maxAge = 300.0f);  // 5 minutes
};

// ============================================================================
// ADAPTIVE GENERATION PARAMETERS
// ============================================================================

/**
 * Dynamic generation parameters that respond to player behavior
 */
struct AdaptiveGenerationConfig {
    // Terrain generation
    float terrainComplexity;        // 0.0 = simple, 1.0 = highly complex
    float caveSystemDensity;        // Frequency of cave networks
    float mineralDistribution;     // Rarity vs abundance of resources
    float geologicalInstability;   // Frequency of unstable formations
    
    // Material properties
    float materialHardness;         // Overall material resistance
    float explosiveMaterialRatio;  // Proportion of volatile materials
    float structuralComplexity;    // Complexity of load-bearing structures
    float resourceAccessibility;   // How easy resources are to reach
    
    // Environmental challenges
    float weatherIntensity;         // Severity of weather events
    float seismicActivity;          // Frequency of earthquakes
    float volcanicActivity;         // Volcanic eruption likelihood
    float climaticInstability;     // Rate of climate change
    
    // Biological factors
    float ecosystemComplexity;      // Biodiversity and interaction complexity
    float evolutionRate;            // Speed of evolutionary changes
    float extinctionRisk;          // Likelihood of species extinction
    float adaptationPressure;      // Environmental stress on life
    
    AdaptiveGenerationConfig()
        : terrainComplexity(0.5f), caveSystemDensity(0.3f), mineralDistribution(0.5f), geologicalInstability(0.2f),
          materialHardness(0.5f), explosiveMaterialRatio(0.1f), structuralComplexity(0.5f), resourceAccessibility(0.7f),
          weatherIntensity(0.3f), seismicActivity(0.1f), volcanicActivity(0.05f), climaticInstability(0.2f),
          ecosystemComplexity(0.5f), evolutionRate(0.3f), extinctionRisk(0.1f), adaptationPressure(0.3f) {}
};

// ============================================================================
// EMERGENT NARRATIVE SYSTEM
// ============================================================================

/**
 * Narrative event types generated by player actions
 */
enum class NarrativeEventType {
    // Geological narratives
    MOUNTAIN_COLLAPSE,
    VALLEY_FORMATION,
    CANYON_CARVING,
    ISLAND_CREATION,
    CONTINENTAL_DRIFT,
    
    // Ecological narratives
    MASS_EXTINCTION,
    EVOLUTIONARY_LEAP,
    ECOSYSTEM_COLLAPSE,
    SPECIES_MIGRATION,
    ADAPTATION_SUCCESS,
    
    // Climatic narratives
    ICE_AGE_ONSET,
    GREENHOUSE_EFFECT,
    ATMOSPHERIC_CHANGE,
    OCEAN_ACIDIFICATION,
    WEATHER_PATTERN_SHIFT,
    
    // Catastrophic narratives
    ASTEROID_IMPACT,
    SUPERVOLCANO_ERUPTION,
    GLOBAL_FLOOD,
    ATMOSPHERIC_IGNITION,
    TECTONIC_CATASTROPHE,
    
    // Technological narratives
    CIVILIZATION_RISE,
    TECHNOLOGICAL_BREAKTHROUGH,
    RESOURCE_DEPLETION,
    ENVIRONMENTAL_RESTORATION,
    SPACE_COLONIZATION
};

/**
 * Narrative event with consequences and story elements
 */
struct NarrativeEvent {
    NarrativeEventType type;
    std::string title;
    std::string description;
    glm::vec3 epicenter;
    float magnitude;
    float duration;
    float timestamp;
    
    // Consequences
    std::vector<glm::vec3> affectedRegions;
    std::map<std::string, float> environmentalChanges;
    std::vector<std::string> speciesAffected;
    float globalImpact;  // 0.0 = local, 1.0 = planetary
    
    // Story progression
    bool triggersSequence;
    std::vector<NarrativeEventType> potentialFollowups;
    float narrativeTension;  // Dramatic tension level
    
    NarrativeEvent(NarrativeEventType t, const std::string& title, const glm::vec3& center)
        : type(t), title(title), epicenter(center), magnitude(1.0f), duration(0.0f),
          timestamp(0.0f), globalImpact(0.0f), triggersSequence(false), narrativeTension(0.5f) {}
};

/**
 * Manages emergent storytelling through environmental consequences
 */
class EmergentNarrativeSystem {
public:
    EmergentNarrativeSystem();
    
    void processPlayerAction(const PlayerAction& action, const AdaptiveGenerationConfig& config);
    void update(float deltaTime);
    
    // Narrative queries
    std::vector<NarrativeEvent> getActiveNarratives() const;
    std::vector<NarrativeEvent> getRecentNarratives(float timeWindow) const;
    float getNarrativeTension() const { return currentNarrativeTension; }
    
    // Story progression
    void triggerNarrativeEvent(NarrativeEventType type, const glm::vec3& location, float magnitude);
    void concludeNarrativeEvent(const NarrativeEvent& event);
    
    // Configuration
    void setNarrativeIntensity(float intensity) { narrativeIntensity = intensity; }
    void setEnableGlobalEvents(bool enable) { enableGlobalEvents = enable; }
    
private:
    std::vector<NarrativeEvent> activeNarratives;
    std::vector<NarrativeEvent> narrativeHistory;
    
    float currentNarrativeTension;
    float narrativeIntensity;
    bool enableGlobalEvents;
    
    // Narrative generation
    void generateNarrativeFromAction(const PlayerAction& action);
    void updateNarrativeConsequences(float deltaTime);
    void checkForNarrativeChains();
    
    // Story templates
    NarrativeEvent createGeologicalNarrative(const PlayerAction& action);
    NarrativeEvent createEcologicalNarrative(const PlayerAction& action);
    NarrativeEvent createClimaticNarrative(const PlayerAction& action);
    NarrativeEvent createCatastrophicNarrative(const PlayerAction& action);
    
    // Consequence calculation
    void calculateEnvironmentalConsequences(NarrativeEvent& event);
    void calculateSpeciesImpact(NarrativeEvent& event);
    void calculateGlobalEffects(NarrativeEvent& event);
};

// ============================================================================
// AI DIRECTOR MAIN CLASS
// ============================================================================

/**
 * AI Director that monitors player actions and dynamically adjusts world generation
 * Implements emergent narrative through environmental consequences
 */
class VoxelAIDirector {
public:
    VoxelAIDirector(VoxelWorldManager& worldManager, VoxelPlanetGenerator& planetGen,
                    DestructionManager& destructionMgr, VoxelClimateSimulator& climateSim);
    ~VoxelAIDirector();
    
    // Player action recording
    void recordPlayerAction(PlayerActionType type, const glm::vec3& location, 
                           float magnitude, const std::vector<uint8_t>& materialsInvolved = {});
    void recordDestructionEvent(const DestructionEvent& event);
    void recordClimateEvent(const std::string& eventType, const glm::vec3& location, float intensity);
    
    // Main update loop
    void update(float deltaTime);
    
    // Generation parameter queries
    AdaptiveGenerationConfig getCurrentGenerationConfig() const { return currentConfig; }
    float getAdaptiveParameter(const std::string& parameterName) const;
    void setAdaptiveParameter(const std::string& parameterName, float value);
    
    // Narrative interface
    std::vector<NarrativeEvent> getCurrentNarratives() const;
    float getNarrativeTension() const;
    void triggerNarrativeEvent(NarrativeEventType type, const glm::vec3& location, float magnitude);
    
    // Director configuration
    void setAdaptationRate(float rate) { adaptationRate = rate; }
    void setNarrativeMode(bool enable) { enableNarrativeMode = enable; }
    void setDifficultyScaling(bool enable) { enableDifficultyScaling = enable; }
    void setPlayerChallengeLevel(float level) { targetChallengeLevel = level; }
    
    // Statistics and analysis
    struct DirectorStats {
        uint32_t totalPlayerActions;
        float averageActionMagnitude;
        float currentAdaptationLevel;
        uint32_t activeNarratives;
        float playerEngagement;
        float worldComplexity;
    };
    DirectorStats getStatistics() const;
    
private:
    // Core systems
    VoxelWorldManager& worldManager;
    VoxelPlanetGenerator& planetGenerator;
    DestructionManager& destructionManager;
    VoxelClimateSimulator& climateSimulator;
    
    // AI Director components
    std::unique_ptr<PlayerBehaviorAnalyzer> behaviorAnalyzer;
    std::unique_ptr<EmergentNarrativeSystem> narrativeSystem;
    
    // Adaptive configuration
    AdaptiveGenerationConfig currentConfig;
    AdaptiveGenerationConfig targetConfig;
    float adaptationRate;
    
    // Director state
    bool enableNarrativeMode;
    bool enableDifficultyScaling;
    float targetChallengeLevel;
    float currentChallengeLevel;
    
    // Timing and scheduling
    float lastAdaptationTime;
    float adaptationInterval;
    std::queue<std::pair<float, std::function<void()>>> scheduledEvents;
    
    // Statistics
    mutable DirectorStats stats;
    mutable std::mutex statsMutex;
    
    // Core adaptation algorithms
    void adaptGenerationParameters();
    void updateTargetConfiguration();
    void interpolateConfiguration(float deltaTime);
    
    // Player behavior response
    void respondToDestructionFocus();
    void respondToConstructionFocus();
    void respondToEnvironmentalFocus();
    void respondToScalePreference();
    
    // Challenge scaling
    void adjustDifficultyLevel();
    void createChallengeOpportunities();
    void balanceResourceAvailability();
    
    // Environmental storytelling
    void triggerEnvironmentalConsequences(const PlayerAction& action);
    void createLongTermConsequences(const std::vector<PlayerAction>& actionHistory);
    void updateGlobalEnvironmentalState();
    
    // Specialized responses
    void handleExplosivePlayerStyle();
    void handleConstructivePlayerStyle();
    void handleExploratoryPlayerStyle();
    void handleChaosPlayerStyle();
    
    // World modification interface
    void modifyChunkGeneration(const ChunkPos& chunkPos, const AdaptiveGenerationConfig& config);
    void adjustMaterialDistribution(const glm::vec3& center, float radius, const AdaptiveGenerationConfig& config);
    void createGeologicalFeatures(const glm::vec3& location, const std::string& featureType);
    
    // Event scheduling
    void scheduleEvent(float delay, std::function<void()> event);
    void processScheduledEvents(float currentTime);
    
    // Utility functions
    float calculatePlayerEngagement() const;
    float calculateWorldComplexity() const;
    void updateStatistics();
    
    // Configuration interpolation
    void lerpConfigParameter(float& current, float target, float rate, float deltaTime);
    void applyConfigurationToSystems();
}; 