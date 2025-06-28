#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

enum class GameMode {
    EXPLORATION,
    SURVIVAL,
    BUILDING,
    MINING
};

enum class ResourceType {
    WATER,
    MINERALS,
    ENERGY,
    BIOMASS,
    RARE_METALS
};

struct Resource {
    ResourceType type;
    float amount;
    std::string name;
    glm::vec3 color;
};

struct PlayerStats {
    float health = 100.0f;
    float oxygen = 100.0f;
    float energy = 100.0f;
    float temperature = 20.0f; // Celsius
    glm::vec3 position;
    float altitude;
    std::string currentBiome;
};

struct Mission {
    std::string title;
    std::string description;
    bool completed = false;
    float progress = 0.0f;
};

class GameState {
public:
    GameState();
    
    void update(float deltaTime, const glm::vec3& playerPosition, float elevation);
    void switchMode(GameMode newMode);
    
    // Resource management
    void addResource(ResourceType type, float amount);
    float getResource(ResourceType type) const;
    bool spendResource(ResourceType type, float amount);
    
    // Player status
    const PlayerStats& getPlayerStats() const { return playerStats; }
    void updatePlayerPosition(const glm::vec3& position, float elevation);
    
    // Mission system
    void addMission(const std::string& title, const std::string& description);
    void updateMissionProgress(int missionIndex, float progress);
    const std::vector<Mission>& getMissions() const { return missions; }
    
    // Game mode
    GameMode getCurrentMode() const { return currentMode; }
    std::string getCurrentModeString() const;
    
    // Environmental effects
    void updateEnvironmentalEffects(float elevation, const std::string& biome);
    
    // Time and day/night cycle
    float getTimeOfDay() const { return timeOfDay; }
    bool isNight() const { return timeOfDay > 18.0f || timeOfDay < 6.0f; }
    
private:
    GameMode currentMode;
    PlayerStats playerStats;
    std::vector<Resource> resources;
    std::vector<Mission> missions;
    
    float timeOfDay; // 0-24 hours
    float dayLength; // Real seconds per game day
    
    void initializeResources();
    void updatePlayerVitals(float deltaTime);
    std::string determineBiome(float elevation);
}; 