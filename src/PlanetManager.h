#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include "LifeForm.h"

enum class ElementType {
    WATER,          // Essential for life
    CARBON,         // Organic compounds
    OXYGEN,         // Atmosphere and respiration
    NITROGEN,       // Atmosphere and proteins
    PHOSPHORUS,     // DNA and energy
    SULFUR,         // Proteins and energy
    IRON,           // Core and metabolism
    SILICON,        // Geology and technology
    RARE_EARTH      // Advanced civilization
};

enum class LifeStage {
    STERILE,        // No life
    PRIMORDIAL,     // Basic organic compounds
    MICROBIAL,      // Single-celled life
    MULTICELLULAR,  // Complex organisms
    PLANT_LIFE,     // Photosynthesis
    ANIMAL_LIFE,    // Mobile organisms
    INTELLIGENT,    // Tool use
    CIVILIZATION,   // Technology
    SPACE_FARING    // Interplanetary
};

enum class ClimateZone {
    ARCTIC,         // Very cold
    TEMPERATE,      // Moderate
    TROPICAL,       // Warm and humid
    DESERT,         // Hot and dry
    OCEANIC         // Water-dominated
};

struct Element {
    ElementType type;
    float abundance;        // 0.0 to 1.0
    std::string name;
    glm::vec3 color;
    std::string description;
};

struct RegionalConditions {
    float temperature;      // -50 to 100 Celsius
    float humidity;         // 0.0 to 1.0
    float atmosphere;       // 0.0 to 2.0 (Earth = 1.0)
    float radiation;        // 0.0 to 2.0 (Earth = 1.0)
    float magneticField;    // 0.0 to 2.0 (Earth = 1.0)
    ClimateZone zone;
    std::map<ElementType, float> elementConcentration;
};

struct GlobalConditions {
    float solarEnergy;      // 0.5 to 2.0 (Earth = 1.0)
    float axialTilt;        // 0 to 45 degrees
    float dayLength;        // 12 to 48 hours
    float yearLength;       // 200 to 800 days
    float gravity;          // 0.5 to 2.0 (Earth = 1.0)
    float volcanism;        // 0.0 to 2.0
    float tectonics;        // 0.0 to 2.0
    float asteroidActivity; // 0.0 to 2.0
};

struct LifeProgress {
    LifeStage currentStage;
    float stageProgress;    // 0.0 to 1.0
    float totalBiomass;
    float diversity;        // Species count
    float intelligence;     // 0.0 to 1.0
    float technology;       // 0.0 to 1.0
    std::vector<std::string> achievements;
};

class PlanetManager {
public:
    PlanetManager();
    
    void update(float deltaTime);
    
    // Global planet management
    void setGlobalCondition(const std::string& condition, float value);
    float getGlobalCondition(const std::string& condition) const;
    
    // Regional management (by biome/location)
    void setRegionalCondition(ClimateZone zone, const std::string& condition, float value);
    float getRegionalCondition(ClimateZone zone, const std::string& condition) const;
    
    // Element management
    void addElement(ElementType type, float amount);
    float getElementAbundance(ElementType type) const;
    void distributeElement(ElementType type, ClimateZone zone, float amount);
    
    // Life evolution
    const LifeProgress& getLifeProgress() const { return lifeProgress; }
    void accelerateEvolution(float factor);
    void triggerEvent(const std::string& eventName);
    
    // Discovery system
    bool discoverElement(const glm::vec3& position, float elevation);
    std::vector<std::string> getRecentDiscoveries() const;
    
    // UI/Display helpers
    std::string getCurrentStageDescription() const;
    std::vector<std::string> getAvailableActions() const;
    std::string getElementDescription(ElementType type) const;
    
    // Planet statistics
    float getPlanetHabitability() const;
    float getLifeSupportability() const;
    float getTechnologicalPotential() const;
    float getTimeAcceleration() const { return timeAcceleration; }
    
    // Life evolution access
    const LifeEvolution& getLifeEvolution() const { return lifeEvolution; }
    void triggerLifeEmergence();
    std::vector<std::string> getLifeFormDescriptions() const;

private:
    GlobalConditions globalConditions;
    std::map<ClimateZone, RegionalConditions> regionalConditions;
    std::vector<Element> elements;
    LifeProgress lifeProgress;
    std::vector<std::string> recentDiscoveries;
    LifeEvolution lifeEvolution;
    
    float timeAcceleration;
    float planetAge; // In millions of years
    bool lifeHasEmerged;
    
    void initializeElements();
    void initializeConditions();
    void updateLifeEvolution(float deltaTime);
    void checkEvolutionTriggers();
    void calculateHabitability();
    
    // Evolution helpers
    bool canAdvanceToStage(LifeStage nextStage) const;
    float getStageRequirement(LifeStage stage, const std::string& requirement) const;
    void unlockAchievement(const std::string& achievement);
}; 