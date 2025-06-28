#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <random>
#include "PlanetManager.h"

enum class StarType {
    MAIN_SEQUENCE_M,    // Red dwarf
    MAIN_SEQUENCE_K,    // Orange dwarf
    MAIN_SEQUENCE_G,    // Yellow dwarf (Sun-like)
    MAIN_SEQUENCE_F,    // Yellow-white
    MAIN_SEQUENCE_A,    // White
    GIANT_RED,          // Red giant
    GIANT_BLUE,         // Blue giant
    WHITE_DWARF,        // White dwarf
    NEUTRON_STAR        // Neutron star
};

struct StarData {
    StarType type;
    float mass;             // Solar masses
    float luminosity;       // Solar luminosities
    float temperature;      // Kelvin
    float radius;           // Solar radii
    glm::vec3 color;       // RGB color for rendering
    float lifespan;        // Billions of years
    std::string name;
};

struct OrbitData {
    float semiMajorAxis;    // AU
    float eccentricity;     // 0-1
    float inclination;      // Degrees
    float orbitalPeriod;    // Years
    bool isHabitable;       // Within habitable zone
    float hillSphere;       // Gravitational influence radius
};

struct PlanetaryBody {
    std::string name;
    PlanetManager* planetManager;
    OrbitData orbit;
    
    // Physical characteristics
    float mass;             // Earth masses
    float radius;           // Earth radii
    float density;          // g/cm³
    float escapeVelocity;   // km/s
    
    // Atmospheric data
    float atmosphericPressure;  // Earth atmospheres
    float greenhouseEffect;     // Temperature boost from atmosphere
    
    // Moons and rings
    std::vector<PlanetaryBody*> moons;
    bool hasRings;
    float ringInnerRadius;
    float ringOuterRadius;
    
    // Life potential
    float habitabilityScore;    // 0-1, based on multiple factors
    bool hasLife;
    float biodiversityIndex;
};

class StellarSystem {
public:
    StellarSystem(uint64_t seed);
    ~StellarSystem();
    
    // Core generation
    void generateSystem();
    void resolveOrbitalMechanics();
    void calculateHabitability();
    void initializePlanets();
    
    // Simulation updates
    void update(float deltaTime);
    void updateOrbitalPositions(float time);
    
    // Getters
    const StarData& getStar() const { return star; }
    const std::vector<PlanetaryBody*>& getPlanets() const { return planets; }
    PlanetaryBody* getCurrentPlanet() const { return currentPlanet; }
    float getSystemAge() const { return systemAge; }
    
    // Player interaction
    void focusPlanet(int planetIndex);
    void triggerStellarEvent(const std::string& eventType);
    std::vector<std::string> getSystemStatus() const;
    
    // God powers at system level
    void alterStellarLuminosity(float factor);
    void triggerSupernovaEvent();
    void introduceCometShower(int planetIndex);
    void createArtificialPlanet(float orbitRadius);
    
private:
    uint64_t systemSeed;
    std::mt19937_64 rng;
    
    StarData star;
    std::vector<PlanetaryBody*> planets;
    PlanetaryBody* currentPlanet;
    
    float systemAge;        // Billions of years
    float timeAcceleration;
    
    // Generation helpers
    StarData generateStar();
    PlanetaryBody* generatePlanet(float orbitRadius, int planetIndex);
    void generateMoons(PlanetaryBody* planet);
    float calculateHillSphere(float planetMass, float orbitRadius, float starMass);
    float calculateRocheLimit(float planetMass, float planetRadius, float moonDensity);
    
    // Orbital mechanics
    glm::vec3 calculatePlanetPosition(const OrbitData& orbit, float time);
    void checkOrbitalStability();
    void resolvePlanetaryCollisions();
    
    // Habitability calculation
    float calculateHabitableZone(float stellarLuminosity);
    float calculateTidalHeating(const PlanetaryBody& planet);
    float calculateAtmosphericRetention(const PlanetaryBody& planet);
    
    // System evolution
    void evolveStellarProperties(float deltaTime);
    void simulateGalacticEvents(float deltaTime);
    void updatePlanetaryInteractions(float deltaTime);
    
    // Helper methods
    std::string getStarTypeName(StarType type) const;
    std::string generateStarName();
    std::string generatePlanetName(int index);
}; 