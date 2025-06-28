#include "LifeForm.h"
#include "PlanetManager.h"
#include <algorithm>
#include <iostream>
#include <cmath>

LifeEvolution::LifeEvolution() : rng(std::random_device{}()) {
    evolutionSpeed = 1.0f;
    mutationPressure = 0.1f;
    environmentalStress = 0.0f;
    timeScale = 1.0f;
}

void LifeEvolution::update(float deltaTime, const GlobalConditions& conditions) {
    float adjustedDelta = deltaTime * timeScale;
    
    // Update environmental stress based on conditions
    environmentalStress = 0.0f;
    if (conditions.solarEnergy < 0.7f || conditions.solarEnergy > 1.4f) environmentalStress += 0.3f;
    if (conditions.volcanism > 1.5f) environmentalStress += 0.4f;
    if (conditions.tectonics > 1.5f) environmentalStress += 0.2f;
    
    // Update all ecosystems
    updateEcosystems(adjustedDelta);
    
    // Evolve organisms
    for (auto& organism : organisms) {
        // Apply environmental pressure
        applyEvolutionaryPressure(organism, environmentalStress * adjustedDelta);
        
        // Update population based on fitness
        float growthRate = organism.reproductionRate * organism.fitness;
        organism.population = std::max(0LL, (long long)(organism.population * (1.0f + growthRate * adjustedDelta)));
        
        // Update biomass
        organism.biomass = organism.population * organism.size * organism.density;
        
        // Evolve intelligence if complex enough
        if (organism.complexity >= LifeComplexity::SPECIALIZED_ORGANS) {
            evolveIntelligence(organism);
        }
    }
    
    // Handle resource competition
    handleResourceCompetition();
    
    // Simulate speciation and extinction
    if (std::uniform_real_distribution<float>(0, 1)(rng) < 0.1f * adjustedDelta) {
        simulateSpeciation();
    }
    
    if (std::uniform_real_distribution<float>(0, 1)(rng) < 0.05f * adjustedDelta) {
        simulateExtinction();
    }
}

void LifeEvolution::introduceLife(const std::map<ElementType, float>& elements, 
                                 const GlobalConditions& conditions) {
    
    // Determine life chemistry based on available elements
    LifeChemistry chemistry = determineChemistry(elements);
    
    std::cout << "Introducing life with chemistry: ";
    switch (chemistry) {
        case LifeChemistry::CARBON_WATER: std::cout << "Carbon-Water (Earth-like)"; break;
        case LifeChemistry::SILICON_AMMONIA: std::cout << "Silicon-Ammonia"; break;
        case LifeChemistry::CRYSTAL_LATTICE: std::cout << "Crystalline"; break;
        case LifeChemistry::PLASMA_ENERGY: std::cout << "Plasma Energy"; break;
        case LifeChemistry::METAL_SULFUR: std::cout << "Metallic-Sulfur"; break;
        case LifeChemistry::HYBRID_SYNTHETIC: std::cout << "Hybrid Synthetic"; break;
    }
    std::cout << std::endl;
    
    // Create initial organism
    Organism primordial;
    primordial.chemistry = chemistry;
    primordial.complexity = LifeComplexity::PRIMAL_SOUP;
    primordial.metabolism = selectMetabolism(conditions, elements);
    primordial.reproduction = Reproduction::BINARY_FISSION;
    
    // Initialize genome
    primordial.genome.codeType = selectGeneticCode(chemistry, 0.1f);
    primordial.genome.stability = 0.3f;
    primordial.genome.adaptability = 0.8f;
    primordial.genome.complexity = 0.1f;
    
    // Create initial genes based on chemistry
    for (auto& [element, abundance] : elements) {
        if (abundance > 0.1f) {
            primordial.genome.genes.push_back(createRandomGene(element));
        }
    }
    
    // Set physical traits
    primordial.size = 0.001f; // Microscopic
    primordial.color = getChemistryColor(chemistry);
    primordial.density = 1.0f;
    primordial.mobility = 0.1f;
    primordial.intelligence = 0.0f;
    primordial.socialness = 0.0f;
    
    // Set environmental requirements
    primordial.temperatureRange[0] = -50.0f;
    primordial.temperatureRange[1] = 150.0f;
    primordial.pressureRange[0] = 0.1f;
    primordial.pressureRange[1] = 10.0f;
    primordial.radiationTolerance = 0.5f;
    
    // Set element needs based on chemistry
    switch (chemistry) {
        case LifeChemistry::CARBON_WATER:
            primordial.elementNeeds[ElementType::CARBON] = 0.8f;
            primordial.elementNeeds[ElementType::WATER] = 0.9f;
            primordial.elementNeeds[ElementType::OXYGEN] = 0.6f;
            break;
        case LifeChemistry::SILICON_AMMONIA:
            primordial.elementNeeds[ElementType::SILICON] = 0.8f;
            primordial.elementNeeds[ElementType::NITROGEN] = 0.7f;
            break;
        case LifeChemistry::CRYSTAL_LATTICE:
            primordial.elementNeeds[ElementType::SILICON] = 0.6f;
            primordial.elementNeeds[ElementType::RARE_EARTH] = 0.5f;
            break;
        case LifeChemistry::PLASMA_ENERGY:
            primordial.elementNeeds[ElementType::IRON] = 0.4f;
            primordial.radiationTolerance = 2.0f;
            break;
        case LifeChemistry::METAL_SULFUR:
            primordial.elementNeeds[ElementType::IRON] = 0.7f;
            primordial.elementNeeds[ElementType::SULFUR] = 0.6f;
            break;
    }
    
    // Set evolutionary traits
    primordial.fitness = 0.5f;
    primordial.reproductionRate = 0.3f;
    primordial.mutationRate = 0.2f;
    primordial.competitiveness = 0.1f;
    
    // Set initial population
    primordial.population = 1000000; // 1 million initial organisms
    primordial.biomass = primordial.population * primordial.size * primordial.density;
    
    organisms.push_back(primordial);
    
    // Create initial ecosystems
    createEcosystemNiches(elements);
}

LifeChemistry LifeEvolution::determineChemistry(const std::map<ElementType, float>& elements) {
    float carbon = elements.count(ElementType::CARBON) ? elements.at(ElementType::CARBON) : 0.0f;
    float water = elements.count(ElementType::WATER) ? elements.at(ElementType::WATER) : 0.0f;
    float silicon = elements.count(ElementType::SILICON) ? elements.at(ElementType::SILICON) : 0.0f;
    float iron = elements.count(ElementType::IRON) ? elements.at(ElementType::IRON) : 0.0f;
    float rareEarth = elements.count(ElementType::RARE_EARTH) ? elements.at(ElementType::RARE_EARTH) : 0.0f;
    
    // Determine most likely chemistry based on element abundance
    if (carbon > 0.3f && water > 0.4f) {
        return LifeChemistry::CARBON_WATER;
    } else if (silicon > 0.4f && iron < 0.3f) {
        return LifeChemistry::SILICON_AMMONIA;
    } else if (silicon > 0.3f && rareEarth > 0.2f) {
        return LifeChemistry::CRYSTAL_LATTICE;
    } else if (iron > 0.5f) {
        return LifeChemistry::METAL_SULFUR;
    } else if (rareEarth > 0.3f) {
        return LifeChemistry::PLASMA_ENERGY;
    } else {
        return LifeChemistry::HYBRID_SYNTHETIC;
    }
}

GeneticCode LifeEvolution::selectGeneticCode(LifeChemistry chemistry, float complexity) {
    switch (chemistry) {
        case LifeChemistry::CARBON_WATER:
            return complexity > 0.5f ? GeneticCode::DNA_DOUBLE_HELIX : GeneticCode::RNA_SINGLE;
        case LifeChemistry::SILICON_AMMONIA:
            return GeneticCode::CHEMICAL_GRADIENTS;
        case LifeChemistry::CRYSTAL_LATTICE:
            return GeneticCode::CRYSTAL_MATRIX;
        case LifeChemistry::PLASMA_ENERGY:
            return complexity > 0.7f ? GeneticCode::QUANTUM_FIELD : GeneticCode::MAGNETIC_DOMAINS;
        case LifeChemistry::METAL_SULFUR:
            return GeneticCode::MAGNETIC_DOMAINS;
        default:
            return GeneticCode::DNA_DOUBLE_HELIX;
    }
}

Metabolism LifeEvolution::selectMetabolism(const GlobalConditions& conditions, 
                                          const std::map<ElementType, float>& elements) {
    
    // Weight different metabolisms based on conditions
    std::vector<std::pair<Metabolism, float>> weights;
    
    weights.push_back({Metabolism::PHOTOSYNTHESIS, conditions.solarEnergy * 2.0f});
    weights.push_back({Metabolism::CHEMOSYNTHESIS, 1.0f});
    weights.push_back({Metabolism::THERMOSYNTHESIS, conditions.volcanism * 1.5f});
    weights.push_back({Metabolism::RADIOSYNTHESIS, conditions.asteroidActivity * 1.2f});
    weights.push_back({Metabolism::ELECTROSYNTHESIS, conditions.tectonics * 1.0f});
    
    // Advanced metabolisms for high-tech environments
    float complexity = 0.0f;
    for (auto& [element, abundance] : elements) {
        if (element == ElementType::RARE_EARTH) complexity += abundance;
    }
    
    if (complexity > 0.3f) {
        weights.push_back({Metabolism::GRAVITATIONAL, complexity * 0.5f});
        weights.push_back({Metabolism::QUANTUM_VACUUM, complexity * 0.3f});
    }
    
    // Select weighted random metabolism
    float totalWeight = 0.0f;
    for (auto& [meta, weight] : weights) totalWeight += weight;
    
    float random = std::uniform_real_distribution<float>(0, totalWeight)(rng);
    float cumulative = 0.0f;
    
    for (auto& [meta, weight] : weights) {
        cumulative += weight;
        if (random <= cumulative) return meta;
    }
    
    return Metabolism::CHEMOSYNTHESIS; // Fallback
}

Gene LifeEvolution::createRandomGene(ElementType baseElement) {
    Gene gene;
    gene.name = "Gene_" + std::to_string(organisms.size()) + "_" + std::to_string(rng());
    gene.expression = std::uniform_real_distribution<float>(0.1f, 0.9f)(rng);
    gene.mutationRate = std::uniform_real_distribution<float>(0.01f, 0.1f)(rng);
    gene.requiredElement = baseElement;
    
    // Create random traits
    int numTraits = std::uniform_int_distribution<int>(3, 8)(rng);
    for (int i = 0; i < numTraits; ++i) {
        gene.traits.push_back(std::uniform_real_distribution<float>(0.0f, 1.0f)(rng));
    }
    
    return gene;
}

void LifeEvolution::simulateSpeciation() {
    if (organisms.empty()) return;
    
    // Find most successful organism to speciate
    auto maxIt = std::max_element(organisms.begin(), organisms.end(),
        [](const Organism& a, const Organism& b) {
            return a.population < b.population;
        });
    
    if (maxIt->population > 100000) { // Minimum population for speciation
        Organism newSpecies = mutateOrganism(*maxIt, 0.3f);
        newSpecies.population = maxIt->population / 4; // Split population
        maxIt->population = maxIt->population * 3 / 4;
        
        organisms.push_back(newSpecies);
        
        std::cout << "SPECIATION: New species evolved from " << 
                     getOrganismDescription(*maxIt) << std::endl;
    }
}

void LifeEvolution::simulateExtinction() {
    // Remove organisms with very low population or fitness
    auto it = std::remove_if(organisms.begin(), organisms.end(),
        [](const Organism& org) {
            return org.population < 1000 || org.fitness < 0.1f;
        });
    
    if (it != organisms.end()) {
        std::cout << "EXTINCTION: " << (organisms.end() - it) << " species went extinct" << std::endl;
        organisms.erase(it, organisms.end());
    }
}

Organism LifeEvolution::mutateOrganism(const Organism& parent, float mutationStrength) {
    Organism mutant = parent;
    
    // Mutate physical traits
    auto mutateFloat = [&](float& value, float range) {
        float mutation = std::normal_distribution<float>(0.0f, range * mutationStrength)(rng);
        value = std::max(0.0f, std::min(1.0f, value + mutation));
    };
    
    mutateFloat(mutant.size, 0.1f);
    mutateFloat(mutant.mobility, 0.1f);
    mutateFloat(mutant.intelligence, 0.05f);
    mutateFloat(mutant.socialness, 0.1f);
    
    // Mutate color
    mutant.color.r += std::normal_distribution<float>(0.0f, 0.1f * mutationStrength)(rng);
    mutant.color.g += std::normal_distribution<float>(0.0f, 0.1f * mutationStrength)(rng);
    mutant.color.b += std::normal_distribution<float>(0.0f, 0.1f * mutationStrength)(rng);
    mutant.color = glm::clamp(mutant.color, 0.0f, 1.0f);
    
    // Mutate genome
    mutant.genome = evolveGenome(parent.genome, {});
    
    // Possibly evolve complexity
    if (std::uniform_real_distribution<float>(0, 1)(rng) < 0.1f * mutationStrength) {
        int currentComplexity = static_cast<int>(mutant.complexity);
        if (currentComplexity < static_cast<int>(LifeComplexity::TRANSCENDENT)) {
            mutant.complexity = static_cast<LifeComplexity>(currentComplexity + 1);
            std::cout << "EVOLUTION: Organism evolved to " << currentComplexity + 1 << " complexity" << std::endl;
        }
    }
    
    return mutant;
}

glm::vec3 LifeEvolution::getChemistryColor(LifeChemistry chemistry) {
    switch (chemistry) {
        case LifeChemistry::CARBON_WATER: return glm::vec3(0.2f, 0.8f, 0.3f); // Green
        case LifeChemistry::SILICON_AMMONIA: return glm::vec3(0.7f, 0.7f, 0.9f); // Light blue
        case LifeChemistry::CRYSTAL_LATTICE: return glm::vec3(0.9f, 0.8f, 0.9f); // Crystal white
        case LifeChemistry::PLASMA_ENERGY: return glm::vec3(1.0f, 0.6f, 0.0f); // Orange energy
        case LifeChemistry::METAL_SULFUR: return glm::vec3(0.8f, 0.6f, 0.2f); // Metallic gold
        case LifeChemistry::HYBRID_SYNTHETIC: return glm::vec3(0.6f, 0.4f, 0.8f); // Purple
        default: return glm::vec3(0.5f, 0.5f, 0.5f);
    }
}

std::string LifeEvolution::getOrganismDescription(const Organism& organism) {
    std::string desc;
    
    switch (organism.chemistry) {
        case LifeChemistry::CARBON_WATER: desc += "Carbon-based "; break;
        case LifeChemistry::SILICON_AMMONIA: desc += "Silicon-based "; break;
        case LifeChemistry::CRYSTAL_LATTICE: desc += "Crystalline "; break;
        case LifeChemistry::PLASMA_ENERGY: desc += "Energy-based "; break;
        case LifeChemistry::METAL_SULFUR: desc += "Metallic "; break;
        case LifeChemistry::HYBRID_SYNTHETIC: desc += "Hybrid "; break;
    }
    
    switch (organism.complexity) {
        case LifeComplexity::PRIMAL_SOUP: desc += "molecules"; break;
        case LifeComplexity::SELF_REPLICATOR: desc += "replicators"; break;
        case LifeComplexity::SIMPLE_CELL: desc += "cells"; break;
        case LifeComplexity::COMPLEX_CELL: desc += "complex cells"; break;
        case LifeComplexity::MULTICELLULAR: desc += "organisms"; break;
        case LifeComplexity::SPECIALIZED_ORGANS: desc += "creatures"; break;
        case LifeComplexity::COLONIAL_MIND: desc += "collective beings"; break;
        case LifeComplexity::TRANSCENDENT: desc += "transcendent entities"; break;
    }
    
    return desc;
}

void LifeEvolution::createEcosystemNiches(const std::map<ElementType, float>& globalElements) {
    ecosystems.clear();
    
    // Create different environmental niches
    EcosystemNiche oceanic;
    oceanic.name = "Oceanic";
    oceanic.temperature = 15.0f;
    oceanic.pressure = 1.0f;
    oceanic.radiation = 0.1f;
    oceanic.availableElements = globalElements;
    oceanic.carryingCapacity = 1000000000; // 1 billion
    
    EcosystemNiche terrestrial;
    terrestrial.name = "Terrestrial";
    terrestrial.temperature = 25.0f;
    terrestrial.pressure = 1.0f;
    terrestrial.radiation = 0.3f;
    terrestrial.availableElements = globalElements;
    terrestrial.carryingCapacity = 500000000; // 500 million
    
    EcosystemNiche volcanic;
    volcanic.name = "Volcanic";
    volcanic.temperature = 80.0f;
    volcanic.pressure = 1.2f;
    volcanic.radiation = 0.8f;
    volcanic.availableElements = globalElements;
    volcanic.carryingCapacity = 100000000; // 100 million
    
    ecosystems = {oceanic, terrestrial, volcanic};
}

void LifeEvolution::updateEcosystems(float deltaTime) {
    for (auto& ecosystem : ecosystems) {
        ecosystem.inhabitants.clear();
        
        // Assign organisms to ecosystems based on environmental requirements
        for (auto& organism : organisms) {
            bool canSurvive = true;
            
            if (ecosystem.temperature < organism.temperatureRange[0] || 
                ecosystem.temperature > organism.temperatureRange[1]) {
                canSurvive = false;
            }
            
            if (ecosystem.pressure < organism.pressureRange[0] || 
                ecosystem.pressure > organism.pressureRange[1]) {
                canSurvive = false;
            }
            
            if (ecosystem.radiation > organism.radiationTolerance) {
                canSurvive = false;
            }
            
            if (canSurvive) {
                ecosystem.inhabitants.push_back(&organism);
            }
        }
        
        // Calculate resource competition
        long long totalPopulation = 0;
        for (auto* org : ecosystem.inhabitants) {
            totalPopulation += org->population;
        }
        
        ecosystem.resourceCompetition = static_cast<float>(totalPopulation) / ecosystem.carryingCapacity;
        
        // Apply competition pressure
        if (ecosystem.resourceCompetition > 1.0f) {
            for (auto* org : ecosystem.inhabitants) {
                org->fitness *= (1.0f / ecosystem.resourceCompetition);
            }
        }
    }
}

// Placeholder implementations for remaining methods
Genome LifeEvolution::evolveGenome(const Genome& parent, const GlobalConditions& conditions) {
    Genome evolved = parent;
    // Add genome evolution logic here
    return evolved;
}

void LifeEvolution::evolveIntelligence(Organism& organism) {
    if (organism.intelligence < 1.0f) {
        organism.intelligence += 0.001f; // Slow intelligence evolution
    }
}

void LifeEvolution::applyEvolutionaryPressure(Organism& organism, float pressure) {
    organism.mutationRate += pressure * 0.1f;
    organism.fitness = std::max(0.1f, organism.fitness - pressure * 0.1f);
}

void LifeEvolution::handleResourceCompetition() {
    // Basic resource competition logic
    for (size_t i = 0; i < organisms.size(); ++i) {
        for (size_t j = i + 1; j < organisms.size(); ++j) {
            // Organisms with similar needs compete
            float competition = 0.0f;
            for (auto& [element, need1] : organisms[i].elementNeeds) {
                if (organisms[j].elementNeeds.count(element)) {
                    float need2 = organisms[j].elementNeeds[element];
                    competition += std::min(need1, need2);
                }
            }
            
            if (competition > 0.5f) {
                // Apply competition pressure
                float pressure = competition * 0.1f;
                organisms[i].fitness -= pressure;
                organisms[j].fitness -= pressure;
            }
        }
    }
}

float LifeEvolution::getTotalBiomass() const {
    float total = 0.0f;
    for (const auto& org : organisms) {
        total += org.biomass;
    }
    return total;
}

float LifeEvolution::getAverageComplexity() const {
    if (organisms.empty()) return 0.0f;
    
    float total = 0.0f;
    for (const auto& org : organisms) {
        total += static_cast<float>(org.complexity);
    }
    return total / organisms.size();
}

std::string LifeEvolution::getEvolutionStatus() const {
    if (organisms.empty()) return "Sterile";
    
    float avgComplexity = getAverageComplexity();
    if (avgComplexity < 1.0f) return "Primordial";
    if (avgComplexity < 3.0f) return "Cellular";
    if (avgComplexity < 5.0f) return "Multicellular";
    if (avgComplexity < 6.0f) return "Complex Life";
    if (avgComplexity < 7.0f) return "Intelligent";
    return "Transcendent";
} 