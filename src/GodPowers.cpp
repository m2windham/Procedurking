#include "GodPowers.h"
#include <iostream>

GodPowerSystem::GodPowerSystem() {
    initializePowers();
}

void GodPowerSystem::initializePowers() {
    defineGeologicalPowers();
    defineAtmosphericPowers();
    defineBiologicalPowers();
    defineStellarPowers();
    defineCosmicPowers();
    defineTemporalPowers();
}

void GodPowerSystem::defineGeologicalPowers() {
    // Increase Volcanism - High impact geological power
    GodPower volcanism;
    volcanism.name = "Awaken the Fire";
    volcanism.description = "Dramatically increase planetary volcanism";
    volcanism.cost = PowerCost::HIGH;
    volcanism.category = PowerCategory::GEOLOGICAL;
    volcanism.cooldown = 30.0f;
    volcanism.firstOrderEffect = "Adds CO2/dust to atmosphere; creates new land/mountains";
    volcanism.cascadingEffects = "Short-term cooling (dust), long-term warming (CO2); new rain shadows; isolated ecosystems";
    volcanism.emergentNarrative = "The Age of Fire and Ash - leading to hothouse planet or extremophile evolution";
    volcanism.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerIncreaseVolcanism(planet, system, magnitude);
    };
    powers.push_back(volcanism);
    
    // Summon Meteor - Medium cost impact event
    GodPower meteor;
    meteor.name = "Celestial Impact";
    meteor.description = "Summon a meteor to impact the planet";
    meteor.cost = PowerCost::MEDIUM;
    meteor.category = PowerCategory::GEOLOGICAL;
    meteor.cooldown = 15.0f;
    meteor.firstOrderEffect = "Creates crater; adds water/dust; kicks up debris";
    meteor.cascadingEffects = "New seas/lakes; climate change; potential mass extinction";
    meteor.emergentNarrative = "The Great Impact - transformation or catastrophe";
    meteor.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerSummonMeteor(planet, system, magnitude);
    };
    powers.push_back(meteor);
}

void GodPowerSystem::defineAtmosphericPowers() {
    // Atmospheric Manipulation
    GodPower atmosphere;
    atmosphere.name = "Breathe Life";
    atmosphere.description = "Alter the planet's atmospheric composition";
    atmosphere.cost = PowerCost::MEDIUM;
    atmosphere.category = PowerCategory::ATMOSPHERIC;
    atmosphere.cooldown = 20.0f;
    atmosphere.firstOrderEffect = "Changes atmospheric pressure and composition";
    atmosphere.cascadingEffects = "Global temperature shifts; weather pattern changes; habitability changes";
    atmosphere.emergentNarrative = "The Great Atmospheric Shift - enabling or destroying life";
    atmosphere.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerAlterAtmosphere(planet, system, magnitude);
    };
    powers.push_back(atmosphere);
}

void GodPowerSystem::defineBiologicalPowers() {
    // Evolution Boost
    GodPower evolution;
    evolution.name = "Spark of Evolution";
    evolution.description = "Accelerate evolutionary processes";
    evolution.cost = PowerCost::LOW;
    evolution.category = PowerCategory::BIOLOGICAL;
    evolution.cooldown = 5.0f;
    evolution.firstOrderEffect = "Increases mutation rates and evolutionary pressure";
    evolution.cascadingEffects = "Rapid speciation; adaptation to new niches; intelligence emergence";
    evolution.emergentNarrative = "The Evolutionary Leap - rapid diversification of life";
    evolution.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerBoostEvolution(planet, system, magnitude);
    };
    powers.push_back(evolution);
    
    // Monolith of Intelligence
    GodPower monolith;
    monolith.name = "Monolith of Transcendence";
    monolith.description = "Grant sapience to a promising species";
    monolith.cost = PowerCost::ULTIMATE;
    monolith.category = PowerCategory::BIOLOGICAL;
    monolith.cooldown = 120.0f;
    monolith.firstOrderEffect = "Dramatically boosts intelligence of target species";
    monolith.cascadingEffects = "Technology development; civilization emergence; ecosystem manipulation";
    monolith.emergentNarrative = "The Awakening - birth of a new intelligent civilization";
    monolith.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerCreateMonolith(planet, system, magnitude);
    };
    monolith.canUse = [](const PlanetManager* planet, const StellarSystem* system) {
        // Can only use if complex life exists
        const auto& progress = planet->getLifeProgress();
        return progress.currentStage >= LifeStage::ANIMAL_LIFE;
    };
    powers.push_back(monolith);
}

void GodPowerSystem::defineStellarPowers() {
    // Stellar Manipulation
    GodPower stellar;
    stellar.name = "Solar Flare";
    stellar.description = "Alter the star's energy output";
    stellar.cost = PowerCost::HIGH;
    stellar.category = PowerCategory::STELLAR;
    stellar.cooldown = 60.0f;
    stellar.firstOrderEffect = "Changes stellar luminosity and radiation";
    stellar.cascadingEffects = "System-wide climate changes; atmospheric loss; radiation effects";
    stellar.emergentNarrative = "The Solar Storm - stellar fury reshapes worlds";
    stellar.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerAlterStellarOutput(planet, system, magnitude);
    };
    powers.push_back(stellar);
}

void GodPowerSystem::defineCosmicPowers() {
    // Supernova - Ultimate destructive power
    GodPower supernova;
    supernova.name = "Stellar Death";
    supernova.description = "Trigger the star's death in a supernova";
    supernova.cost = PowerCost::ULTIMATE;
    supernova.category = PowerCategory::COSMIC;
    supernova.cooldown = 300.0f;
    supernova.firstOrderEffect = "Destroys the star and sterilizes the system";
    supernova.cascadingEffects = "Total system destruction; heavy element dispersal; potential new star formation";
    supernova.emergentNarrative = "The Final Chapter - death and rebirth of worlds";
    supernova.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerSupernova(planet, system, magnitude);
    };
    powers.push_back(supernova);
}

void GodPowerSystem::defineTemporalPowers() {
    // Time Acceleration
    GodPower time;
    time.name = "Temporal Flux";
    time.description = "Accelerate the flow of time";
    time.cost = PowerCost::LOW;
    time.category = PowerCategory::TEMPORAL;
    time.cooldown = 0.0f; // No cooldown for time powers
    time.firstOrderEffect = "Speeds up all planetary processes";
    time.cascadingEffects = "Rapid evolution; geological changes; stellar aging";
    time.emergentNarrative = "The Time Stream - watching eons pass in moments";
    time.execute = [this](PlanetManager* planet, StellarSystem* system, float magnitude) {
        powerTimeAcceleration(planet, system, magnitude);
    };
    powers.push_back(time);
}

bool GodPowerSystem::usePower(const std::string& powerName, PlanetManager* planet, 
                             StellarSystem* system, float magnitude) {
    
    // Find the power
    auto powerIt = std::find_if(powers.begin(), powers.end(),
        [&powerName](const GodPower& power) {
            return power.name == powerName;
        });
    
    if (powerIt == powers.end()) {
        std::cout << "Power '" << powerName << "' not found!" << std::endl;
        return false;
    }
    
    // Check cooldown
    if (!isPowerReady(powerName)) {
        std::cout << "Power '" << powerName << "' is on cooldown!" << std::endl;
        return false;
    }
    
    // Check prerequisites
    if (powerIt->canUse && !powerIt->canUse(planet, system)) {
        std::cout << "Prerequisites not met for power '" << powerName << "'!" << std::endl;
        return false;
    }
    
    // Execute the power
    std::cout << "=== DIVINE INTERVENTION ===" << std::endl;
    std::cout << "Using power: " << powerName << std::endl;
    std::cout << "Effect: " << powerIt->firstOrderEffect << std::endl;
    
    powerIt->execute(planet, system, magnitude);
    
    // Set cooldown
    powerCooldowns[powerName] = powerIt->cooldown;
    
    // Record in narrative history
    std::string narrativeEvent = generateNarrativeEvent(powerName, {});
    narrativeHistory.push_back(narrativeEvent);
    
    std::cout << "Cascading effects: " << powerIt->cascadingEffects << std::endl;
    std::cout << "Narrative: " << powerIt->emergentNarrative << std::endl;
    std::cout << "=========================" << std::endl;
    
    return true;
}

void GodPowerSystem::update(float deltaTime) {
    // Update cooldowns
    for (auto& [powerName, cooldown] : powerCooldowns) {
        cooldown = std::max(0.0f, cooldown - deltaTime);
    }
}

bool GodPowerSystem::isPowerReady(const std::string& powerName) const {
    auto it = powerCooldowns.find(powerName);
    return (it == powerCooldowns.end()) || (it->second <= 0.0f);
}

std::vector<GodPower> GodPowerSystem::getAvailablePowers(const PlanetManager* planet, 
                                                        const StellarSystem* system) const {
    std::vector<GodPower> available;
    
    for (const auto& power : powers) {
        if (isPowerReady(power.name)) {
            if (!power.canUse || power.canUse(planet, system)) {
                available.push_back(power);
            }
        }
    }
    
    return available;
}

// Power implementations
void GodPowerSystem::powerIncreaseVolcanism(PlanetManager* planet, StellarSystem* system, float magnitude) {
    float currentVolcanism = planet->getGlobalCondition("volcanism");
    float newVolcanism = currentVolcanism + (magnitude * 0.5f);
    planet->setGlobalCondition("volcanism", newVolcanism);
    
    std::cout << "Volcanism increased from " << currentVolcanism << " to " << newVolcanism << std::endl;
}

void GodPowerSystem::powerSummonMeteor(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Add water and trigger temporary cooling
    planet->addElement(ElementType::WATER, magnitude * 0.1f);
    
    float currentTemp = planet->getGlobalCondition("solarEnergy");
    planet->setGlobalCondition("solarEnergy", currentTemp * (1.0f - magnitude * 0.1f));
    
    std::cout << "Meteor impact! Added water and caused temporary cooling." << std::endl;
}

void GodPowerSystem::powerAlterAtmosphere(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Boost atmospheric elements
    planet->addElement(ElementType::OXYGEN, magnitude * 0.05f);
    planet->addElement(ElementType::NITROGEN, magnitude * 0.03f);
    
    std::cout << "Atmospheric composition altered. Breathability improved." << std::endl;
}

void GodPowerSystem::powerBoostEvolution(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Accelerate evolution by boosting time
    planet->accelerateEvolution(planet->getTimeAcceleration() * (1.0f + magnitude));
    
    std::cout << "Evolution accelerated! Life will adapt faster." << std::endl;
}

void GodPowerSystem::powerCreateMonolith(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Trigger life emergence if not present, or boost intelligence
    planet->triggerLifeEmergence();
    
    // Add rare earth elements for technology
    planet->addElement(ElementType::RARE_EARTH, magnitude * 0.1f);
    
    std::cout << "Monolith erected! Intelligence sparked in the most promising species." << std::endl;
}

void GodPowerSystem::powerAlterStellarOutput(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Alter solar energy reaching the planet
    float currentSolar = planet->getGlobalCondition("solarEnergy");
    float newSolar = currentSolar * (1.0f + magnitude * 0.2f);
    planet->setGlobalCondition("solarEnergy", newSolar);
    
    std::cout << "Stellar output changed! Solar energy: " << newSolar << "x" << std::endl;
}

void GodPowerSystem::powerSupernova(PlanetManager* planet, StellarSystem* system, float magnitude) {
    // Catastrophic system destruction
    planet->setGlobalCondition("solarEnergy", 0.0f);
    planet->setGlobalCondition("volcanism", 10.0f);
    
    std::cout << "SUPERNOVA! The star has died in a brilliant explosion!" << std::endl;
    std::cout << "All life in the system has been sterilized..." << std::endl;
}

void GodPowerSystem::powerTimeAcceleration(PlanetManager* planet, StellarSystem* system, float magnitude) {
    planet->accelerateEvolution(magnitude * 2.0f);
    std::cout << "Time flows faster! Evolution and geology accelerated." << std::endl;
}

std::string GodPowerSystem::generateNarrativeEvent(const std::string& powerName, 
                                                  const std::vector<PowerEffect>& effects) const {
    return "The gods stirred... " + powerName + " was unleashed upon the world.";
}

std::vector<std::string> GodPowerSystem::getPlanetaryHistory() const {
    return narrativeHistory;
} 