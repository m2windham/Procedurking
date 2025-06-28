#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <algorithm>
#include "PlanetManager.h"
#include "PlanetarySystem.h"

enum class PowerCost {
    LOW,        // Frequent use powers
    MEDIUM,     // Moderate cost powers  
    HIGH,       // Expensive, rare powers
    ULTIMATE    // Game-changing powers
};

enum class PowerCategory {
    GEOLOGICAL,     // Volcanism, tectonics, impacts
    ATMOSPHERIC,    // Climate, weather, atmosphere
    BIOLOGICAL,     // Life, evolution, extinction
    STELLAR,        // Star properties, radiation
    COSMIC,         // System-wide effects
    TEMPORAL        // Time manipulation
};

struct PowerEffect {
    std::string name;
    std::string description;
    float magnitude;
    float duration;
    bool isPermanent;
};

struct GodPower {
    std::string name;
    std::string description;
    PowerCost cost;
    PowerCategory category;
    float cooldown;
    
    // Systemic effects description
    std::string firstOrderEffect;
    std::string cascadingEffects;
    std::string emergentNarrative;
    
    // Function to execute the power
    std::function<void(PlanetManager*, StellarSystem*, float)> execute;
    
    // Prerequisites (optional)
    std::function<bool(const PlanetManager*, const StellarSystem*)> canUse;
};

class GodPowerSystem {
public:
    GodPowerSystem();
    
    // Power management
    void initializePowers();
    bool usePower(const std::string& powerName, PlanetManager* planet, StellarSystem* system, float magnitude = 1.0f);
    void update(float deltaTime);
    
    // Power availability
    std::vector<GodPower> getAvailablePowers(const PlanetManager* planet, const StellarSystem* system) const;
    bool isPowerReady(const std::string& powerName) const;
    float getPowerCooldown(const std::string& powerName) const;
    
    // Narrative tracking
    void recordPowerUse(const std::string& powerName, const std::vector<PowerEffect>& effects);
    std::vector<std::string> getPlanetaryHistory() const;
    std::string generateNarrativeEvent(const std::string& powerName, const std::vector<PowerEffect>& effects) const;
    
    // God power categories
    std::vector<GodPower> getGeologicalPowers() const;
    std::vector<GodPower> getAtmosphericPowers() const;
    std::vector<GodPower> getBiologicalPowers() const;
    std::vector<GodPower> getStellarPowers() const;
    std::vector<GodPower> getCosmicPowers() const;
    
private:
    std::vector<GodPower> powers;
    std::map<std::string, float> powerCooldowns;
    std::vector<std::string> narrativeHistory;
    
    // Power definitions
    void defineGeologicalPowers();
    void defineAtmosphericPowers();
    void defineBiologicalPowers();
    void defineStellarPowers();
    void defineCosmicPowers();
    void defineTemporalPowers();
    
    // Specific power implementations
    void powerIncreaseVolcanism(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerSummonMeteor(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerAlterAtmosphere(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerTriggerIceAge(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerBoostEvolution(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerMassExtinction(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerCreateMonolith(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerAlterStellarOutput(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerSupernova(PlanetManager* planet, StellarSystem* system, float magnitude);
    void powerTimeAcceleration(PlanetManager* planet, StellarSystem* system, float magnitude);
    
    // Effect calculation helpers
    std::vector<PowerEffect> calculateVolcanismEffects(float magnitude) const;
    std::vector<PowerEffect> calculateMeteorEffects(float magnitude) const;
    std::vector<PowerEffect> calculateAtmosphericEffects(float magnitude) const;
    std::vector<PowerEffect> calculateEvolutionEffects(float magnitude) const;
    
    // Narrative generation
    std::string generateGeologicalNarrative(const std::string& powerName, float magnitude) const;
    std::string generateBiologicalNarrative(const std::string& powerName, float magnitude) const;
    std::string generateAtmosphericNarrative(const std::string& powerName, float magnitude) const;
    
    // Cascading effect simulation
    void simulateCascadingEffects(const std::string& powerName, PlanetManager* planet, 
                                 float magnitude, float deltaTime);
    void triggerFeedbackLoops(PlanetManager* planet, const std::string& triggerType);
}; 