#include "PlanetarySystem.h"
#include <algorithm>
#include <iostream>
#include <cmath>

StellarSystem::StellarSystem(uint64_t seed) 
    : systemSeed(seed), rng(seed), currentPlanet(nullptr), systemAge(0.0f), timeAcceleration(1.0f) {
    generateSystem();
}

StellarSystem::~StellarSystem() {
    for (auto* planet : planets) {
        delete planet->planetManager;
        for (auto* moon : planet->moons) {
            delete moon->planetManager;
            delete moon;
        }
        delete planet;
    }
}

void StellarSystem::generateSystem() {
    // Generate the central star
    star = generateStar();
    
    // Generate planets
    int numPlanets = std::uniform_int_distribution<int>(2, 8)(rng);
    
    // Generate initial orbital radii (logarithmic distribution)
    std::vector<float> orbits;
    for (int i = 0; i < numPlanets; ++i) {
        float minOrbit = 0.1f + i * 0.3f;  // Minimum safe distance
        float maxOrbit = 50.0f;            // Outer system limit
        float logMin = std::log(minOrbit);
        float logMax = std::log(maxOrbit);
        float logOrbit = std::uniform_real_distribution<float>(logMin, logMax)(rng);
        orbits.push_back(std::exp(logOrbit));
    }
    
    // Sort orbits to ensure proper ordering
    std::sort(orbits.begin(), orbits.end());
    
    // Generate planets at these orbits
    for (int i = 0; i < numPlanets; ++i) {
        PlanetaryBody* planet = generatePlanet(orbits[i], i);
        planets.push_back(planet);
    }
    
    // Resolve orbital mechanics and conflicts
    resolveOrbitalMechanics();
    
    // Calculate habitability for all bodies
    calculateHabitability();
    
    // Initialize planet managers
    initializePlanets();
    
    // Focus on the most habitable planet initially
    auto mostHabitable = std::max_element(planets.begin(), planets.end(),
        [](const PlanetaryBody* a, const PlanetaryBody* b) {
            return a->habitabilityScore < b->habitabilityScore;
        });
    
    if (mostHabitable != planets.end()) {
        currentPlanet = *mostHabitable;
    }
    
    std::cout << "Generated stellar system with " << numPlanets << " planets around a " 
              << getStarTypeName(star.type) << " star" << std::endl;
}

StarData StellarSystem::generateStar() {
    StarData starData;
    
    // Generate star type based on realistic distribution
    float typeRoll = std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
    
    if (typeRoll < 0.76f) {
        starData.type = StarType::MAIN_SEQUENCE_M;  // Red dwarf (most common)
        starData.mass = std::uniform_real_distribution<float>(0.08f, 0.6f)(rng);
        starData.temperature = 2300.0f + starData.mass * 1700.0f;
        starData.luminosity = std::pow(starData.mass, 3.5f);
        starData.color = glm::vec3(1.0f, 0.3f, 0.1f);
        starData.lifespan = 10.0f + starData.mass * 90.0f;
    } else if (typeRoll < 0.88f) {
        starData.type = StarType::MAIN_SEQUENCE_K;  // Orange dwarf
        starData.mass = std::uniform_real_distribution<float>(0.6f, 0.9f)(rng);
        starData.temperature = 3700.0f + starData.mass * 1300.0f;
        starData.luminosity = std::pow(starData.mass, 4.0f);
        starData.color = glm::vec3(1.0f, 0.7f, 0.4f);
        starData.lifespan = 5.0f + starData.mass * 15.0f;
    } else if (typeRoll < 0.96f) {
        starData.type = StarType::MAIN_SEQUENCE_G;  // Sun-like
        starData.mass = std::uniform_real_distribution<float>(0.9f, 1.3f)(rng);
        starData.temperature = 5200.0f + starData.mass * 800.0f;
        starData.luminosity = std::pow(starData.mass, 4.0f);
        starData.color = glm::vec3(1.0f, 1.0f, 0.8f);
        starData.lifespan = 8.0f + starData.mass * 2.0f;
    } else {
        starData.type = StarType::MAIN_SEQUENCE_F;  // Hot star
        starData.mass = std::uniform_real_distribution<float>(1.3f, 2.0f)(rng);
        starData.temperature = 6000.0f + starData.mass * 1000.0f;
        starData.luminosity = std::pow(starData.mass, 4.0f);
        starData.color = glm::vec3(0.9f, 0.9f, 1.0f);
        starData.lifespan = 1.0f + starData.mass * 2.0f;
    }
    
    starData.radius = std::sqrt(starData.luminosity) / std::pow(starData.temperature / 5778.0f, 2.0f);
    starData.name = generateStarName();
    
    return starData;
}

PlanetaryBody* StellarSystem::generatePlanet(float orbitRadius, int planetIndex) {
    PlanetaryBody* planet = new PlanetaryBody();
    
    // Generate orbital characteristics
    planet->orbit.semiMajorAxis = orbitRadius;
    planet->orbit.eccentricity = std::uniform_real_distribution<float>(0.0f, 0.3f)(rng);
    planet->orbit.inclination = std::normal_distribution<float>(0.0f, 5.0f)(rng);
    planet->orbit.orbitalPeriod = std::sqrt(std::pow(orbitRadius, 3.0f) / star.mass);
    
    // Generate physical characteristics based on distance from star
    if (orbitRadius < 0.5f) {
        // Rocky inner planet
        planet->mass = std::uniform_real_distribution<float>(0.1f, 2.0f)(rng);
        planet->radius = 0.3f + planet->mass * 0.7f;
        planet->density = 3.0f + std::uniform_real_distribution<float>(0.0f, 3.0f)(rng);
        planet->atmosphericPressure = std::uniform_real_distribution<float>(0.0f, 5.0f)(rng);
    } else if (orbitRadius < 3.0f) {
        // Terrestrial zone
        planet->mass = std::uniform_real_distribution<float>(0.5f, 3.0f)(rng);
        planet->radius = 0.5f + planet->mass * 0.5f;
        planet->density = 4.0f + std::uniform_real_distribution<float>(0.0f, 2.0f)(rng);
        planet->atmosphericPressure = std::uniform_real_distribution<float>(0.1f, 10.0f)(rng);
    } else {
        // Gas giant zone
        planet->mass = std::uniform_real_distribution<float>(10.0f, 300.0f)(rng);
        planet->radius = 3.0f + planet->mass * 0.1f;
        planet->density = 0.5f + std::uniform_real_distribution<float>(0.0f, 1.5f)(rng);
        planet->atmosphericPressure = std::uniform_real_distribution<float>(50.0f, 1000.0f)(rng);
    }
    
    // Calculate derived properties
    planet->escapeVelocity = std::sqrt(2.0f * 6.67e-11f * planet->mass * 5.97e24f / (planet->radius * 6.37e6f)) / 1000.0f;
    planet->greenhouseEffect = std::min(100.0f, planet->atmosphericPressure * 10.0f);
    
    // Generate name
    planet->name = generatePlanetName(planetIndex);
    
    // Initialize other properties
    planet->hasRings = false;
    planet->hasLife = false;
    planet->biodiversityIndex = 0.0f;
    planet->planetManager = nullptr;  // Will be created later
    
    return planet;
}

void StellarSystem::resolveOrbitalMechanics() {
    // Calculate Hill spheres for all planets
    for (auto* planet : planets) {
        planet->orbit.hillSphere = calculateHillSphere(planet->mass, planet->orbit.semiMajorAxis, star.mass);
    }
    
    // Check for moons vs collisions
    for (size_t i = 0; i < planets.size(); ++i) {
        for (size_t j = i + 1; j < planets.size(); ++j) {
            float distance = std::abs(planets[i]->orbit.semiMajorAxis - planets[j]->orbit.semiMajorAxis);
            
            // If smaller body is within larger body's Hill sphere, make it a moon
            if (distance < planets[i]->orbit.hillSphere && planets[j]->mass < planets[i]->mass) {
                // Convert planet j to moon of planet i
                planets[i]->moons.push_back(planets[j]);
                planets.erase(planets.begin() + j);
                j--; // Adjust index after removal
                
                std::cout << "Planet " << planets[j]->name << " became a moon of " << planets[i]->name << std::endl;
            } else if (distance < planets[j]->orbit.hillSphere && planets[i]->mass < planets[j]->mass) {
                // Convert planet i to moon of planet j
                planets[j]->moons.push_back(planets[i]);
                planets.erase(planets.begin() + i);
                i--; // Adjust index after removal
                
                std::cout << "Planet " << planets[i]->name << " became a moon of " << planets[j]->name << std::endl;
            }
        }
    }
    
    // Check Roche limit for ring formation
    for (auto* planet : planets) {
        for (auto it = planet->moons.begin(); it != planet->moons.end();) {
            float rocheLimit = calculateRocheLimit(planet->mass, planet->radius, (*it)->density);
            float moonOrbitRadius = 2.5f * planet->radius; // Simplified moon orbit
            
            if (moonOrbitRadius < rocheLimit) {
                // Moon is torn apart into rings
                planet->hasRings = true;
                planet->ringInnerRadius = rocheLimit * 0.8f;
                planet->ringOuterRadius = rocheLimit * 2.0f;
                
                std::cout << "Moon " << (*it)->name << " was torn apart to form rings around " << planet->name << std::endl;
                
                delete (*it)->planetManager;
                delete *it;
                it = planet->moons.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void StellarSystem::calculateHabitability() {
    float habitableZoneInner = calculateHabitableZone(star.luminosity) * 0.8f;
    float habitableZoneOuter = calculateHabitableZone(star.luminosity) * 1.5f;
    
    for (auto* planet : planets) {
        float habitability = 0.0f;
        
        // Distance from habitable zone
        if (planet->orbit.semiMajorAxis >= habitableZoneInner && 
            planet->orbit.semiMajorAxis <= habitableZoneOuter) {
            habitability += 0.4f; // Base habitability for being in zone
        } else {
            float distance = std::min(std::abs(planet->orbit.semiMajorAxis - habitableZoneInner),
                                    std::abs(planet->orbit.semiMajorAxis - habitableZoneOuter));
            habitability += std::max(0.0f, 0.4f - distance * 0.1f);
        }
        
        // Mass factor (not too big, not too small)
        if (planet->mass >= 0.5f && planet->mass <= 2.0f) {
            habitability += 0.2f;
        } else {
            habitability += std::max(0.0f, 0.2f - std::abs(planet->mass - 1.0f) * 0.1f);
        }
        
        // Atmospheric pressure
        if (planet->atmosphericPressure >= 0.5f && planet->atmosphericPressure <= 2.0f) {
            habitability += 0.2f;
        } else {
            habitability += std::max(0.0f, 0.2f - std::abs(planet->atmosphericPressure - 1.0f) * 0.1f);
        }
        
        // Orbital eccentricity (lower is better for stable climate)
        habitability += std::max(0.0f, 0.1f - planet->orbit.eccentricity * 0.2f);
        
        // Star type bonus (G and K stars are best for life)
        if (star.type == StarType::MAIN_SEQUENCE_G || star.type == StarType::MAIN_SEQUENCE_K) {
            habitability += 0.1f;
        }
        
        planet->habitabilityScore = std::min(1.0f, habitability);
        planet->orbit.isHabitable = planet->habitabilityScore > 0.5f;
        
        std::cout << "Planet " << planet->name << " habitability: " << 
                     (int)(planet->habitabilityScore * 100) << "%" << std::endl;
    }
}

void StellarSystem::initializePlanets() {
    for (auto* planet : planets) {
        // Only create full planet managers for potentially habitable worlds
        if (planet->habitabilityScore > 0.3f) {
            planet->planetManager = new PlanetManager();
            
            // Set initial conditions based on orbital characteristics
            float solarMultiplier = star.luminosity / (planet->orbit.semiMajorAxis * planet->orbit.semiMajorAxis);
            planet->planetManager->setGlobalCondition("solarEnergy", solarMultiplier);
            
            // Adjust other conditions based on planet properties
            float volcanismLevel = std::max(0.1f, 2.0f - planet->orbit.semiMajorAxis);
            planet->planetManager->setGlobalCondition("volcanism", volcanismLevel);
            
            std::cout << "Initialized planet manager for " << planet->name << std::endl;
        }
    }
}

float StellarSystem::calculateHillSphere(float planetMass, float orbitRadius, float starMass) {
    // Hill sphere radius in AU
    return orbitRadius * std::pow(planetMass / (3.0f * starMass), 1.0f/3.0f);
}

float StellarSystem::calculateRocheLimit(float planetMass, float planetRadius, float moonDensity) {
    // Simplified Roche limit calculation
    float planetDensity = planetMass / (4.0f/3.0f * 3.14159f * planetRadius * planetRadius * planetRadius);
    return 2.44f * planetRadius * std::pow(planetDensity / moonDensity, 1.0f/3.0f);
}

float StellarSystem::calculateHabitableZone(float stellarLuminosity) {
    // Conservative habitable zone calculation (liquid water)
    return std::sqrt(stellarLuminosity); // In AU
}

void StellarSystem::update(float deltaTime) {
    systemAge += deltaTime * timeAcceleration * 0.001f; // Convert to billions of years
    
    // Update current planet if one is focused
    if (currentPlanet && currentPlanet->planetManager) {
        currentPlanet->planetManager->update(deltaTime);
        
        // Update life status
        const auto& lifeProgress = currentPlanet->planetManager->getLifeProgress();
        currentPlanet->hasLife = (lifeProgress.currentStage > LifeStage::STERILE);
        currentPlanet->biodiversityIndex = lifeProgress.diversity / 1000.0f; // Normalize
    }
    
    // Evolve stellar properties over time
    evolveStellarProperties(deltaTime);
    
    // Update orbital positions
    updateOrbitalPositions(systemAge);
}

void StellarSystem::evolveStellarProperties(float deltaTime) {
    // Stars evolve over billions of years
    float ageInBillionYears = systemAge;
    
    // Very slow stellar evolution
    if (ageInBillionYears > star.lifespan * 0.9f) {
        // Star approaching end of life
        if (star.type == StarType::MAIN_SEQUENCE_G) {
            // Sun-like stars become red giants
            star.type = StarType::GIANT_RED;
            star.radius *= 100.0f;
            star.luminosity *= 1000.0f;
            star.temperature *= 0.5f;
            star.color = glm::vec3(1.0f, 0.4f, 0.2f);
            
            std::cout << "STELLAR EVOLUTION: " << star.name << " has become a red giant!" << std::endl;
        }
    }
}

void StellarSystem::updateOrbitalPositions(float time) {
    // Update planet positions for rendering (if needed)
    for (auto* planet : planets) {
        glm::vec3 position = calculatePlanetPosition(planet->orbit, time);
        // Store position for rendering system
    }
}

glm::vec3 StellarSystem::calculatePlanetPosition(const OrbitData& orbit, float time) {
    float angle = 2.0f * 3.14159f * time / orbit.orbitalPeriod;
    float x = orbit.semiMajorAxis * std::cos(angle);
    float y = orbit.semiMajorAxis * std::sin(angle) * std::cos(glm::radians(orbit.inclination));
    float z = orbit.semiMajorAxis * std::sin(angle) * std::sin(glm::radians(orbit.inclination));
    return glm::vec3(x, y, z);
}

void StellarSystem::focusPlanet(int planetIndex) {
    if (planetIndex >= 0 && planetIndex < planets.size()) {
        currentPlanet = planets[planetIndex];
        std::cout << "Focused on planet: " << currentPlanet->name << std::endl;
    }
}

std::vector<std::string> StellarSystem::getSystemStatus() const {
    std::vector<std::string> status;
    
    status.push_back("=== STELLAR SYSTEM STATUS ===");
    status.push_back("Star: " + star.name + " (" + getStarTypeName(star.type) + ")");
    status.push_back("System Age: " + std::to_string(systemAge) + " billion years");
    status.push_back("Planets: " + std::to_string(planets.size()));
    
    for (size_t i = 0; i < planets.size(); ++i) {
        std::string planetInfo = std::to_string(i + 1) + ". " + planets[i]->name;
        planetInfo += " (Habitability: " + std::to_string((int)(planets[i]->habitabilityScore * 100)) + "%)";
        if (planets[i]->hasLife) {
            planetInfo += " [LIFE DETECTED]";
        }
        if (planets[i] == currentPlanet) {
            planetInfo += " [CURRENT]";
        }
        status.push_back(planetInfo);
    }
    
    return status;
}

// Helper functions
std::string StellarSystem::getStarTypeName(StarType type) const {
    switch (type) {
        case StarType::MAIN_SEQUENCE_M: return "Red Dwarf";
        case StarType::MAIN_SEQUENCE_K: return "Orange Dwarf";
        case StarType::MAIN_SEQUENCE_G: return "Yellow Dwarf";
        case StarType::MAIN_SEQUENCE_F: return "White Star";
        case StarType::MAIN_SEQUENCE_A: return "Hot White Star";
        case StarType::GIANT_RED: return "Red Giant";
        case StarType::GIANT_BLUE: return "Blue Giant";
        case StarType::WHITE_DWARF: return "White Dwarf";
        case StarType::NEUTRON_STAR: return "Neutron Star";
        default: return "Unknown";
    }
}

std::string StellarSystem::generateStarName() {
    std::vector<std::string> prefixes = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta"};
    std::vector<std::string> suffixes = {"Centauri", "Orionis", "Draconis", "Lyrae", "Cygni", "Aquilae", "Boötis"};
    
    int prefixIndex = std::uniform_int_distribution<int>(0, prefixes.size() - 1)(rng);
    int suffixIndex = std::uniform_int_distribution<int>(0, suffixes.size() - 1)(rng);
    
    return prefixes[prefixIndex] + " " + suffixes[suffixIndex];
}

std::string StellarSystem::generatePlanetName(int index) {
    std::vector<std::string> names = {
        "Kepler", "Gliese", "Proxima", "Tau Ceti", "Wolf", "Ross", "Lacaille",
        "Groombridge", "Kapteyn", "Barnard", "Luyten", "Kruger"
    };
    
    int nameIndex = std::uniform_int_distribution<int>(0, names.size() - 1)(rng);
    return names[nameIndex] + " " + std::to_string(index + 1);
} 