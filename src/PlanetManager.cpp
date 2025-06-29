#include "PlanetManager.h"
#include <algorithm>
#include <iostream>
#include <random>
#include "Icosphere.h"
#include "TerrainSampler.h"
#include <array>
#include <tuple>

PlanetManager::PlanetManager() 
    : timeAcceleration(1.0f), planetAge(0.0f), lifeHasEmerged(false) {
    initializeElements();
    initializeConditions();
    
    // Start with sterile planet
    lifeProgress.currentStage = LifeStage::STERILE;
    lifeProgress.stageProgress = 0.0f;
    lifeProgress.totalBiomass = 0.0f;
    lifeProgress.diversity = 0.0f;
    lifeProgress.intelligence = 0.0f;
    lifeProgress.technology = 0.0f;
}

void PlanetManager::initializeElements() {
    elements = {
        {ElementType::WATER, 0.3f, "Water", glm::vec3(0.2f, 0.5f, 0.9f), "Essential for all known life"},
        {ElementType::CARBON, 0.1f, "Carbon", glm::vec3(0.2f, 0.2f, 0.2f), "Foundation of organic chemistry"},
        {ElementType::OXYGEN, 0.2f, "Oxygen", glm::vec3(0.8f, 0.9f, 1.0f), "Enables complex metabolism"},
        {ElementType::NITROGEN, 0.4f, "Nitrogen", glm::vec3(0.7f, 0.8f, 0.9f), "Atmospheric buffer and protein building"},
        {ElementType::PHOSPHORUS, 0.05f, "Phosphorus", glm::vec3(0.9f, 0.7f, 0.3f), "DNA backbone and energy storage"},
        {ElementType::SULFUR, 0.08f, "Sulfur", glm::vec3(0.9f, 0.9f, 0.2f), "Protein structure and energy"},
        {ElementType::IRON, 0.15f, "Iron", glm::vec3(0.6f, 0.3f, 0.1f), "Planetary core and biological processes"},
        {ElementType::SILICON, 0.25f, "Silicon", glm::vec3(0.7f, 0.7f, 0.7f), "Geological foundation"},
        {ElementType::RARE_EARTH, 0.02f, "Rare Earth", glm::vec3(0.8f, 0.5f, 0.9f), "Advanced technology catalyst"}
    };
}

void PlanetManager::initializeConditions() {
    // Set Earth-like starting conditions
    globalConditions = {
        1.0f,   // solarEnergy
        23.5f,  // axialTilt
        24.0f,  // dayLength
        365.0f, // yearLength
        1.0f,   // gravity
        0.5f,   // volcanism
        0.8f,   // tectonics
        0.3f    // asteroidActivity
    };
    
    // Initialize regional conditions for each climate zone
    regionalConditions[ClimateZone::ARCTIC] = {
        -20.0f, 0.3f, 1.0f, 1.0f, 1.0f, ClimateZone::ARCTIC, {}
    };
    regionalConditions[ClimateZone::TEMPERATE] = {
        15.0f, 0.6f, 1.0f, 1.0f, 1.0f, ClimateZone::TEMPERATE, {}
    };
    regionalConditions[ClimateZone::TROPICAL] = {
        28.0f, 0.8f, 1.0f, 1.0f, 1.0f, ClimateZone::TROPICAL, {}
    };
    regionalConditions[ClimateZone::DESERT] = {
        35.0f, 0.1f, 1.0f, 1.2f, 1.0f, ClimateZone::DESERT, {}
    };
    regionalConditions[ClimateZone::OCEANIC] = {
        18.0f, 1.0f, 1.0f, 0.8f, 1.0f, ClimateZone::OCEANIC, {}
    };
}

void PlanetManager::update(float deltaTime) {
    planetAge += deltaTime * timeAcceleration * 0.1f; // 0.1 million years per second at 1x speed
    
    // Update life evolution system
    if (lifeHasEmerged) {
        lifeEvolution.update(deltaTime * timeAcceleration, globalConditions);
        
        // Update life progress based on evolution system
        lifeProgress.totalBiomass = lifeEvolution.getTotalBiomass();
        lifeProgress.diversity = static_cast<float>(lifeEvolution.getSpeciesCount());
        
        float avgComplexity = lifeEvolution.getAverageComplexity();
        if (avgComplexity < 1.0f) lifeProgress.currentStage = LifeStage::PRIMORDIAL;
        else if (avgComplexity < 3.0f) lifeProgress.currentStage = LifeStage::MICROBIAL;
        else if (avgComplexity < 4.0f) lifeProgress.currentStage = LifeStage::MULTICELLULAR;
        else if (avgComplexity < 5.0f) lifeProgress.currentStage = LifeStage::PLANT_LIFE;
        else if (avgComplexity < 6.0f) lifeProgress.currentStage = LifeStage::ANIMAL_LIFE;
        else if (avgComplexity < 7.0f) lifeProgress.currentStage = LifeStage::INTELLIGENT;
        else if (avgComplexity < 8.0f) lifeProgress.currentStage = LifeStage::CIVILIZATION;
        else lifeProgress.currentStage = LifeStage::SPACE_FARING;
        
        // Calculate intelligence and technology from organisms
        const auto& organisms = lifeEvolution.getAllOrganisms();
        float maxIntelligence = 0.0f;
        float maxTechnology = 0.0f;
        
        for (const auto& org : organisms) {
            maxIntelligence = std::max(maxIntelligence, org.intelligence);
            if (org.complexity >= LifeComplexity::COLONIAL_MIND) {
                maxTechnology = std::max(maxTechnology, org.intelligence * 0.8f);
            }
        }
        
        lifeProgress.intelligence = maxIntelligence;
        lifeProgress.technology = maxTechnology;
        lifeProgress.stageProgress = avgComplexity / 8.0f;
    } else {
        updateLifeEvolution(deltaTime);
    }
    
    checkEvolutionTriggers();
    calculateHabitability();
}

void PlanetManager::updateLifeEvolution(float deltaTime) {
    if (lifeProgress.currentStage == LifeStage::STERILE && !lifeHasEmerged) {
        // Check if conditions are right for life to emerge
        float habitability = getPlanetHabitability();
        if (habitability > 0.3f && planetAge > 0.5f) {
            triggerLifeEmergence();
        }
        return;
    }
}

bool PlanetManager::canAdvanceToStage(LifeStage nextStage) const {
    switch (nextStage) {
        case LifeStage::PRIMORDIAL:
            return getElementAbundance(ElementType::WATER) > 0.1f && 
                   getElementAbundance(ElementType::CARBON) > 0.05f;
        case LifeStage::MICROBIAL:
            return getElementAbundance(ElementType::PHOSPHORUS) > 0.02f;
        case LifeStage::MULTICELLULAR:
            return getElementAbundance(ElementType::OXYGEN) > 0.1f;
        case LifeStage::PLANT_LIFE:
            return getPlanetHabitability() > 0.4f;
        case LifeStage::ANIMAL_LIFE:
            return getElementAbundance(ElementType::OXYGEN) > 0.15f;
        case LifeStage::INTELLIGENT:
            return lifeProgress.diversity > 100.0f && getPlanetHabitability() > 0.6f;
        case LifeStage::CIVILIZATION:
            return lifeProgress.intelligence > 0.5f && getElementAbundance(ElementType::IRON) > 0.1f;
        case LifeStage::SPACE_FARING:
            return lifeProgress.technology > 0.7f && getElementAbundance(ElementType::RARE_EARTH) > 0.01f;
        default:
            return false;
    }
}

std::string PlanetManager::getCurrentStageDescription() const {
    switch (lifeProgress.currentStage) {
        case LifeStage::STERILE: return "Sterile World";
        case LifeStage::PRIMORDIAL: return "Primordial Soup";
        case LifeStage::MICROBIAL: return "Microbial Life";
        case LifeStage::MULTICELLULAR: return "Multicellular Organisms";
        case LifeStage::PLANT_LIFE: return "Plant Life";
        case LifeStage::ANIMAL_LIFE: return "Animal Life";
        case LifeStage::INTELLIGENT: return "Intelligent Life";
        case LifeStage::CIVILIZATION: return "Civilization";
        case LifeStage::SPACE_FARING: return "Space-Faring Civilization";
        default: return "Unknown";
    }
}

bool PlanetManager::discoverElement(const glm::vec3& position, float elevation) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    
    // Different elements are more likely at different elevations and locations
    ElementType discoveredElement = ElementType::WATER;
    float discoveryChance = 0.1f;
    
    if (elevation < -0.02f) {
        // Deep ocean - water and rare elements
        if (dis(gen) < 0.3f) {
            discoveredElement = ElementType::WATER;
            discoveryChance = 0.4f;
        } else if (dis(gen) < 0.1f) {
            discoveredElement = ElementType::RARE_EARTH;
            discoveryChance = 0.2f;
        }
    } else if (elevation < 0.1f) {
        // Coastal/shallow - carbon and phosphorus
        if (dis(gen) < 0.4f) {
            discoveredElement = ElementType::CARBON;
            discoveryChance = 0.3f;
        } else if (dis(gen) < 0.2f) {
            discoveredElement = ElementType::PHOSPHORUS;
            discoveryChance = 0.25f;
        }
    } else if (elevation < 0.3f) {
        // Land - iron and silicon
        if (dis(gen) < 0.5f) {
            discoveredElement = ElementType::IRON;
            discoveryChance = 0.35f;
        } else {
            discoveredElement = ElementType::SILICON;
            discoveryChance = 0.4f;
        }
    } else {
        // High altitude - rare earth and sulfur
        if (dis(gen) < 0.3f) {
            discoveredElement = ElementType::RARE_EARTH;
            discoveryChance = 0.3f;
        } else {
            discoveredElement = ElementType::SULFUR;
            discoveryChance = 0.25f;
        }
    }
    
    if (dis(gen) < discoveryChance) {
        addElement(discoveredElement, 0.05f);
        
        std::string elementName = getElementDescription(discoveredElement);
        recentDiscoveries.push_back("Discovered " + elementName + "!");
        if (recentDiscoveries.size() > 5) {
            recentDiscoveries.erase(recentDiscoveries.begin());
        }
        
        std::cout << "DISCOVERY: Found " << elementName << "! Abundance increased." << std::endl;
        return true;
    }
    
    return false;
}

void PlanetManager::addElement(ElementType type, float amount) {
    for (auto& element : elements) {
        if (element.type == type) {
            element.abundance = std::min(1.0f, element.abundance + amount);
            break;
        }
    }
}

float PlanetManager::getElementAbundance(ElementType type) const {
    for (const auto& element : elements) {
        if (element.type == type) {
            return element.abundance;
        }
    }
    return 0.0f;
}

std::string PlanetManager::getElementDescription(ElementType type) const {
    for (const auto& element : elements) {
        if (element.type == type) {
            return element.name;
        }
    }
    return "Unknown";
}

float PlanetManager::getPlanetHabitability() const {
    float habitability = 1.0f;
    
    // Temperature range check
    float avgTemp = (regionalConditions.at(ClimateZone::TEMPERATE).temperature + 
                     regionalConditions.at(ClimateZone::TROPICAL).temperature) / 2.0f;
    if (avgTemp < -10.0f || avgTemp > 50.0f) {
        habitability *= 0.5f;
    }
    
    // Atmospheric pressure
    float avgAtmosphere = regionalConditions.at(ClimateZone::TEMPERATE).atmosphere;
    if (avgAtmosphere < 0.5f || avgAtmosphere > 1.5f) {
        habitability *= 0.7f;
    }
    
    // Water availability
    habitability *= getElementAbundance(ElementType::WATER);
    
    // Radiation protection
    float avgMagnetic = regionalConditions.at(ClimateZone::TEMPERATE).magneticField;
    if (avgMagnetic < 0.5f) {
        habitability *= 0.8f;
    }
    
    return std::max(0.0f, std::min(1.0f, habitability));
}

void PlanetManager::setGlobalCondition(const std::string& condition, float value) {
    if (condition == "solarEnergy") globalConditions.solarEnergy = value;
    else if (condition == "gravity") globalConditions.gravity = value;
    else if (condition == "volcanism") globalConditions.volcanism = value;
    else if (condition == "tectonics") globalConditions.tectonics = value;
    else if (condition == "asteroids") globalConditions.asteroidActivity = value;
}

float PlanetManager::getGlobalCondition(const std::string& condition) const {
    if (condition == "solarEnergy") return globalConditions.solarEnergy;
    else if (condition == "gravity") return globalConditions.gravity;
    else if (condition == "volcanism") return globalConditions.volcanism;
    else if (condition == "tectonics") return globalConditions.tectonics;
    else if (condition == "asteroids") return globalConditions.asteroidActivity;
    return 0.0f;
}

void PlanetManager::accelerateEvolution(float factor) {
    timeAcceleration = std::max(0.1f, std::min(10.0f, factor));
    std::cout << "Time acceleration set to " << timeAcceleration << "x" << std::endl;
}

void PlanetManager::checkEvolutionTriggers() {
    // Check for extinction events
    if (globalConditions.asteroidActivity > 1.5f && lifeProgress.currentStage > LifeStage::STERILE) {
        if (planetAge > 100.0f) { // Only after some time
            lifeProgress.totalBiomass *= 0.5f;
            lifeProgress.diversity *= 0.3f;
            std::cout << "EXTINCTION EVENT: Asteroid impact reduces life!" << std::endl;
        }
    }
    
    // Volcanic events can help or hurt
    if (globalConditions.volcanism > 1.5f) {
        addElement(ElementType::SULFUR, 0.02f);
        addElement(ElementType::IRON, 0.01f);
    }
}

void PlanetManager::calculateHabitability() {
    // This is called automatically in update()
}

void PlanetManager::unlockAchievement(const std::string& achievement) {
    if (std::find(lifeProgress.achievements.begin(), lifeProgress.achievements.end(), achievement) == lifeProgress.achievements.end()) {
        lifeProgress.achievements.push_back(achievement);
    }
}

std::vector<std::string> PlanetManager::getRecentDiscoveries() const {
    return recentDiscoveries;
}

std::vector<std::string> PlanetManager::getAvailableActions() const {
    std::vector<std::string> actions;
    actions.push_back("Explore surface to discover elements");
    actions.push_back("Adjust planetary conditions with number keys");
    actions.push_back("Monitor life evolution progress");
    
    if (lifeProgress.currentStage >= LifeStage::MICROBIAL) {
        actions.push_back("Guide evolution by optimizing conditions");
    }
    
    return actions;
}

void PlanetManager::triggerLifeEmergence() {
    if (lifeHasEmerged) return;
    
    // Create element abundance map for life evolution
    std::map<ElementType, float> elementMap;
    for (const auto& element : elements) {
        elementMap[element.type] = element.abundance;
    }
    
    // Initialize life evolution system
    lifeEvolution.introduceLife(elementMap, globalConditions);
    lifeHasEmerged = true;
    
    // Update life progress
    lifeProgress.currentStage = LifeStage::PRIMORDIAL;
    lifeProgress.stageProgress = 0.0f;
    lifeProgress.totalBiomass = 0.001f;
    lifeProgress.diversity = 1.0f;
    
    std::cout << "EVOLUTION: Life has emerged on the planet!" << std::endl;
    unlockAchievement("Life Emergence");
}

std::vector<std::string> PlanetManager::getLifeFormDescriptions() const {
    std::vector<std::string> descriptions;
    
    if (!lifeHasEmerged) {
        descriptions.push_back("No life detected");
        return descriptions;
    }
    
    const auto& organisms = lifeEvolution.getAllOrganisms();
    for (const auto& organism : organisms) {
        std::string desc = "Population: " + std::to_string(organism.population) + " - ";
        
        // Add chemistry description
        switch (organism.chemistry) {
            case LifeChemistry::CARBON_WATER:
                desc += "Carbon-based ";
                break;
            case LifeChemistry::SILICON_AMMONIA:
                desc += "Silicon-based ";
                break;
            case LifeChemistry::CRYSTAL_LATTICE:
                desc += "Crystalline ";
                break;
            case LifeChemistry::PLASMA_ENERGY:
                desc += "Energy-based ";
                break;
            case LifeChemistry::METAL_SULFUR:
                desc += "Metallic ";
                break;
            case LifeChemistry::HYBRID_SYNTHETIC:
                desc += "Hybrid ";
                break;
        }
        
        // Add complexity description
        switch (organism.complexity) {
            case LifeComplexity::PRIMAL_SOUP:
                desc += "molecules";
                break;
            case LifeComplexity::SELF_REPLICATOR:
                desc += "replicators";
                break;
            case LifeComplexity::SIMPLE_CELL:
                desc += "cells";
                break;
            case LifeComplexity::COMPLEX_CELL:
                desc += "complex cells";
                break;
            case LifeComplexity::MULTICELLULAR:
                desc += "organisms";
                break;
            case LifeComplexity::SPECIALIZED_ORGANS:
                desc += "creatures";
                break;
            case LifeComplexity::COLONIAL_MIND:
                desc += "collective beings";
                break;
            case LifeComplexity::TRANSCENDENT:
                desc += "transcendent entities";
                break;
        }
        
        // Add metabolism info
        desc += " (";
        switch (organism.metabolism) {
            case Metabolism::PHOTOSYNTHESIS:
                desc += "photosynthetic";
                break;
            case Metabolism::CHEMOSYNTHESIS:
                desc += "chemosynthetic";
                break;
            case Metabolism::THERMOSYNTHESIS:
                desc += "thermosynthetic";
                break;
            case Metabolism::RADIOSYNTHESIS:
                desc += "radiosynthetic";
                break;
            case Metabolism::ELECTROSYNTHESIS:
                desc += "electrosynthetic";
                break;
            case Metabolism::GRAVITATIONAL:
                desc += "gravitational";
                break;
            case Metabolism::QUANTUM_VACUUM:
                desc += "quantum";
                break;
        }
        desc += ")";
        
        // Add intelligence if significant
        if (organism.intelligence > 0.1f) {
            desc += " - Intelligence: " + std::to_string((int)(organism.intelligence * 100)) + "%";
        }
        
        descriptions.push_back(desc);
    }
    
    // Add ecosystem information
    descriptions.push_back("");
    descriptions.push_back("Evolution Status: " + lifeEvolution.getEvolutionStatus());
    descriptions.push_back("Total Species: " + std::to_string(lifeEvolution.getSpeciesCount()));
    descriptions.push_back("Total Biomass: " + std::to_string(lifeEvolution.getTotalBiomass()));
    
    return descriptions;
}

// Procedural flora and fauna generation stubs

// Plant section types
enum class PlantSectionType {
    ROOT,
    STEM,
    LEAF,
    FLOWER,
    FRUIT,
    BRANCH,
    SEED,
    OTHER
};

struct PlantSection {
    PlantSectionType type;
    float length; // For stem/root/branch
    float width;  // For leaf/flower/fruit
    glm::vec3 color;
    std::string description;
};

struct Plant {
    std::vector<PlantSection> sections;
    glm::vec3 position;
    float height;
    std::string speciesName;
};

static Plant GenerateRandomPlant(const glm::vec3& position, std::mt19937& rng) {
    std::uniform_real_distribution<float> lenDist(0.1f, 2.0f);
    std::uniform_real_distribution<float> widDist(0.05f, 0.5f);
    std::uniform_int_distribution<int> flowerChance(0, 3); // 25% chance for flower
    std::uniform_int_distribution<int> fruitChance(0, 4);  // 20% chance for fruit
    std::uniform_int_distribution<int> branchCountDist(0, 2);
    std::uniform_int_distribution<int> leafCountDist(2, 8);
    std::uniform_int_distribution<int> colorDist(0, 2);
    std::array<glm::vec3, 3> leafColors = { glm::vec3(0.2f, 0.7f, 0.2f), glm::vec3(0.4f, 0.8f, 0.3f), glm::vec3(0.1f, 0.5f, 0.1f) };
    std::array<glm::vec3, 3> flowerColors = { glm::vec3(1.0f, 0.8f, 0.9f), glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(0.9f, 0.5f, 1.0f) };
    std::array<glm::vec3, 3> fruitColors = { glm::vec3(1.0f, 0.3f, 0.2f), glm::vec3(1.0f, 0.7f, 0.2f), glm::vec3(0.7f, 0.3f, 0.1f) };
    Plant plant;
    plant.position = position;
    plant.height = lenDist(rng) + 0.5f;
    plant.speciesName = "ProceduralPlant";
    // Root
    plant.sections.push_back({ PlantSectionType::ROOT, lenDist(rng), widDist(rng), glm::vec3(0.5f, 0.3f, 0.1f), "Root" });
    // Stem
    plant.sections.push_back({ PlantSectionType::STEM, plant.height, widDist(rng), glm::vec3(0.3f, 0.2f, 0.1f), "Stem" });
    // Branches
    int branchCount = branchCountDist(rng);
    for (int b = 0; b < branchCount; ++b) {
        plant.sections.push_back({ PlantSectionType::BRANCH, lenDist(rng), widDist(rng), glm::vec3(0.3f, 0.2f, 0.1f), "Branch" });
    }
    // Leaves
    int leafCount = leafCountDist(rng);
    for (int l = 0; l < leafCount; ++l) {
        plant.sections.push_back({ PlantSectionType::LEAF, lenDist(rng) * 0.3f, widDist(rng), leafColors[colorDist(rng)], "Leaf" });
    }
    // Flower (optional)
    if (flowerChance(rng) == 0) {
        plant.sections.push_back({ PlantSectionType::FLOWER, widDist(rng), widDist(rng), flowerColors[colorDist(rng)], "Flower" });
    }
    // Fruit (optional)
    if (fruitChance(rng) == 0) {
        plant.sections.push_back({ PlantSectionType::FRUIT, widDist(rng), widDist(rng), fruitColors[colorDist(rng)], "Fruit" });
    }
    return plant;
}

void GenerateProceduralPlants(Icosphere* planet, const TerrainConfig& config) {
    std::cout << "[ProceduralGen] Generating plants..." << std::endl;
    const auto& verts = planet->getVertices();
    const auto& elevs = planet->getElevations();
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    int treeCount = 0, bushCount = 0;
    std::vector<Plant> allPlants;
    for (size_t i = 0; i < verts.size(); ++i) {
        float elevation = elevs[i];
        if (elevation > 0.01f && elevation < config.maxElevation * 0.4f) { // Land only
            float plantChance = dist(rng);
            if (plantChance > 0.995f) {
                // Place a tree
                Plant plant = GenerateRandomPlant(verts[i], rng);
                plant.speciesName = "Tree";
                allPlants.push_back(plant);
                ++treeCount;
            } else if (plantChance > 0.99f) {
                // Place a bush (smaller, fewer sections)
                Plant plant = GenerateRandomPlant(verts[i], rng);
                plant.speciesName = "Bush";
                if (plant.sections.size() > 4) plant.sections.resize(4); // Shrub-like
                allPlants.push_back(plant);
                ++bushCount;
            }
        }
    }
    std::cout << "Placed " << treeCount << " trees and " << bushCount << " bushes." << std::endl;
    std::cout << "Total plants: " << allPlants.size() << std::endl;
}

void GenerateProceduralAnimals(Icosphere* planet, const TerrainConfig& config) {
    std::cout << "[ProceduralGen] Generating animals..." << std::endl;
    const auto& verts = planet->getVertices();
    const auto& elevs = planet->getElevations();
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    int herdCount = 0, animalCount = 0;
    for (size_t i = 0; i < verts.size(); ++i) {
        float elevation = elevs[i];
        if (elevation > 0.02f && elevation < config.maxElevation * 0.3f) { // Favor plains/low hills
            float animalChance = dist(rng);
            if (animalChance > 0.999f) {
                // Place a herd
                ++herdCount;
                animalCount += 10 + (int)(dist(rng) * 20); // 10-30 animals per herd
            } else if (animalChance > 0.997f) {
                // Place a lone animal
                ++animalCount;
            }
        }
    }
    std::cout << "Placed " << herdCount << " herds and " << animalCount << " animals." << std::endl;
} 