#include "GameState.h"
#include <algorithm>
#include <iostream>

GameState::GameState() 
    : currentMode(GameMode::EXPLORATION), timeOfDay(8.0f), dayLength(300.0f) { // 5 minute days
    initializeResources();
    
    // Add initial missions
    addMission("First Steps", "Explore the planet surface and find a safe landing zone");
    addMission("Resource Gathering", "Collect 50 units of water and 25 units of minerals");
    addMission("Biome Explorer", "Visit 5 different biomes on the planet");
    addMission("High Altitude", "Reach an elevation above 0.3 units (mountain peaks)");
}

void GameState::initializeResources() {
    resources.clear();
    
    resources.push_back({ResourceType::WATER, 10.0f, "Water", glm::vec3(0.3f, 0.5f, 0.9f)});
    resources.push_back({ResourceType::MINERALS, 5.0f, "Minerals", glm::vec3(0.6f, 0.4f, 0.2f)});
    resources.push_back({ResourceType::ENERGY, 100.0f, "Energy", glm::vec3(1.0f, 0.9f, 0.2f)});
    resources.push_back({ResourceType::BIOMASS, 0.0f, "Biomass", glm::vec3(0.3f, 0.7f, 0.2f)});
    resources.push_back({ResourceType::RARE_METALS, 0.0f, "Rare Metals", glm::vec3(0.8f, 0.7f, 0.9f)});
}

void GameState::update(float deltaTime, const glm::vec3& playerPosition, float elevation) {
    // Update time of day
    timeOfDay += (deltaTime / dayLength) * 24.0f;
    if (timeOfDay >= 24.0f) {
        timeOfDay -= 24.0f;
    }
    
    // Update player position and stats
    updatePlayerPosition(playerPosition, elevation);
    updatePlayerVitals(deltaTime);
    
    // Update environmental effects
    std::string biome = determineBiome(elevation);
    updateEnvironmentalEffects(elevation, biome);
    
    // Update mission progress
    if (!missions.empty()) {
        // Mission 0: First Steps (just being on surface)
        if (elevation > 0.0f && !missions[0].completed) {
            missions[0].progress = 100.0f;
            missions[0].completed = true;
            std::cout << "Mission Complete: " << missions[0].title << std::endl;
        }
        
        // Mission 1: Resource gathering
        if (missions.size() > 1) {
            float waterProgress = std::min(getResource(ResourceType::WATER) / 50.0f, 1.0f);
            float mineralProgress = std::min(getResource(ResourceType::MINERALS) / 25.0f, 1.0f);
            missions[1].progress = (waterProgress + mineralProgress) * 50.0f;
            if (missions[1].progress >= 100.0f && !missions[1].completed) {
                missions[1].completed = true;
                std::cout << "Mission Complete: " << missions[1].title << std::endl;
            }
        }
        
        // Mission 3: High altitude
        if (missions.size() > 3 && elevation > 0.3f && !missions[3].completed) {
            missions[3].progress = 100.0f;
            missions[3].completed = true;
            std::cout << "Mission Complete: " << missions[3].title << std::endl;
        }
    }
}

void GameState::updatePlayerVitals(float deltaTime) {
    // Energy consumption based on activity
    float energyConsumption = 2.0f * deltaTime; // Base consumption
    
    // Environmental effects on vitals
    if (playerStats.altitude > 0.4f) {
        // High altitude - increased energy consumption, reduced oxygen
        energyConsumption *= 1.5f;
        playerStats.oxygen -= 5.0f * deltaTime;
    }
    
    if (playerStats.temperature < -10.0f || playerStats.temperature > 40.0f) {
        // Extreme temperatures
        playerStats.health -= 1.0f * deltaTime;
        energyConsumption *= 1.3f;
    }
    
    // Apply energy consumption
    playerStats.energy = std::max(0.0f, playerStats.energy - energyConsumption);
    
    // Regenerate oxygen slowly when on surface with atmosphere
    if (playerStats.altitude < 0.3f && playerStats.currentBiome != "Ocean") {
        playerStats.oxygen = std::min(100.0f, playerStats.oxygen + 10.0f * deltaTime);
    }
    
    // Health regeneration when well-fed and rested
    if (playerStats.energy > 50.0f && playerStats.oxygen > 70.0f) {
        playerStats.health = std::min(100.0f, playerStats.health + 2.0f * deltaTime);
    }
    
    // Clamp values
    playerStats.health = std::max(0.0f, std::min(100.0f, playerStats.health));
    playerStats.oxygen = std::max(0.0f, std::min(100.0f, playerStats.oxygen));
    playerStats.energy = std::max(0.0f, std::min(100.0f, playerStats.energy));
}

void GameState::updateEnvironmentalEffects(float elevation, const std::string& biome) {
    // Temperature based on elevation and biome
    float baseTemp = 25.0f; // Base temperature
    
    // Altitude effect
    baseTemp -= elevation * 100.0f; // Colder at higher altitudes
    
    // Biome effects
    if (biome == "Ocean") {
        baseTemp += 5.0f; // Oceans moderate temperature
    } else if (biome == "Desert") {
        baseTemp += 15.0f; // Hot deserts
    } else if (biome == "Snow") {
        baseTemp -= 20.0f; // Cold snow areas
    } else if (biome == "Forest") {
        baseTemp += 2.0f; // Forests slightly warmer
    }
    
    // Day/night cycle effect
    if (isNight()) {
        baseTemp -= 10.0f;
    } else {
        baseTemp += 5.0f;
    }
    
    playerStats.temperature = baseTemp;
    playerStats.currentBiome = biome;
}

std::string GameState::determineBiome(float elevation) {
    if (elevation < -0.05f) return "Deep Ocean";
    if (elevation < 0.0f) return "Ocean";
    if (elevation < 0.01f) return "Beach";
    if (elevation < 0.08f) return "Plains";
    if (elevation < 0.15f) return "Forest";
    if (elevation < 0.25f) return "Hills";
    if (elevation < 0.35f) return "Mountains";
    return "Snow";
}

void GameState::switchMode(GameMode newMode) {
    currentMode = newMode;
    std::cout << "Switched to " << getCurrentModeString() << " mode" << std::endl;
}

std::string GameState::getCurrentModeString() const {
    switch (currentMode) {
        case GameMode::EXPLORATION: return "Exploration";
        case GameMode::SURVIVAL: return "Survival";
        case GameMode::BUILDING: return "Building";
        case GameMode::MINING: return "Mining";
        default: return "Unknown";
    }
}

void GameState::addResource(ResourceType type, float amount) {
    for (auto& resource : resources) {
        if (resource.type == type) {
            resource.amount += amount;
            break;
        }
    }
}

float GameState::getResource(ResourceType type) const {
    for (const auto& resource : resources) {
        if (resource.type == type) {
            return resource.amount;
        }
    }
    return 0.0f;
}

bool GameState::spendResource(ResourceType type, float amount) {
    for (auto& resource : resources) {
        if (resource.type == type && resource.amount >= amount) {
            resource.amount -= amount;
            return true;
        }
    }
    return false;
}

void GameState::updatePlayerPosition(const glm::vec3& position, float elevation) {
    playerStats.position = position;
    playerStats.altitude = elevation;
}

void GameState::addMission(const std::string& title, const std::string& description) {
    missions.push_back({title, description, false, 0.0f});
}

void GameState::updateMissionProgress(int missionIndex, float progress) {
    if (missionIndex >= 0 && missionIndex < missions.size()) {
        missions[missionIndex].progress = std::min(100.0f, progress);
        if (missions[missionIndex].progress >= 100.0f) {
            missions[missionIndex].completed = true;
        }
    }
} 