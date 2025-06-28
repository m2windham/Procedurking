#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>
#include <random>

// Forward declarations - these will be defined in PlanetManager.h
enum class ElementType;
struct GlobalConditions;

enum class LifeChemistry {
    CARBON_WATER,       // Earth-like DNA/RNA
    SILICON_AMMONIA,    // Silicon-based in ammonia
    CRYSTAL_LATTICE,    // Crystalline life forms
    PLASMA_ENERGY,      // Energy-based entities
    METAL_SULFUR,       // Metallic organisms
    HYBRID_SYNTHETIC    // Mixed chemistry
};

enum class GeneticCode {
    DNA_DOUBLE_HELIX,   // Standard Earth DNA
    RNA_SINGLE,         // RNA-based genetics
    CRYSTAL_MATRIX,     // Information stored in crystal structures
    QUANTUM_FIELD,      // Quantum entanglement patterns
    MAGNETIC_DOMAINS,   // Magnetic field patterns
    CHEMICAL_GRADIENTS  // Concentration-based information
};

enum class LifeComplexity {
    PRIMAL_SOUP,        // Basic organic compounds
    SELF_REPLICATOR,    // First reproducing molecules
    SIMPLE_CELL,        // Basic cellular life
    COMPLEX_CELL,       // Eukaryotic equivalent
    MULTICELLULAR,      // Simple multicellular
    SPECIALIZED_ORGANS, // Complex body systems
    COLONIAL_MIND,      // Collective intelligence
    TRANSCENDENT       // Beyond physical form
};

enum class Metabolism {
    PHOTOSYNTHESIS,     // Light energy
    CHEMOSYNTHESIS,     // Chemical energy
    THERMOSYNTHESIS,    // Heat energy
    RADIOSYNTHESIS,     // Radiation energy
    ELECTROSYNTHESIS,   // Electrical energy
    GRAVITATIONAL,      // Gravitational fields
    QUANTUM_VACUUM      // Zero-point energy
};

enum class Reproduction {
    BINARY_FISSION,     // Simple splitting
    SEXUAL_GENETIC,     // Genetic recombination
    BUDDING,           // Asexual budding
    SPORE_FORMATION,   // Spore-based
    CRYSTALLIZATION,   // Crystal growth
    ENERGY_TRANSFER,   // Information copying
    QUANTUM_TUNNELING  // Quantum reproduction
};

struct Gene {
    std::string name;
    float expression;           // 0.0 to 1.0 - how active this gene is
    float mutationRate;         // Likelihood to change
    std::vector<float> traits;  // Multiple trait values
    ElementType requiredElement; // What element this gene needs
};

struct Genome {
    GeneticCode codeType;
    std::vector<Gene> genes;
    float stability;            // How resistant to mutation
    float adaptability;         // How quickly it can change
    float complexity;           // How sophisticated the organism
};

struct Organism {
    LifeChemistry chemistry;
    LifeComplexity complexity;
    Metabolism metabolism;
    Reproduction reproduction;
    Genome genome;
    
    // Physical traits
    float size;                 // Scale factor
    glm::vec3 color;           // Visual appearance
    float density;             // How tightly packed
    float mobility;            // Movement capability
    float intelligence;        // Cognitive ability
    float socialness;          // Group behavior tendency
    
    // Environmental requirements
    float temperatureRange[2];  // Min/max temperature
    float pressureRange[2];     // Min/max pressure
    float radiationTolerance;   // Radiation resistance
    std::map<ElementType, float> elementNeeds; // Required elements
    
    // Evolutionary traits
    float fitness;             // Survival capability
    float reproductionRate;    // How fast it reproduces
    float mutationRate;        // How fast it evolves
    float competitiveness;     // Resource competition
    
    // Population data
    long long population;      // Current population
    float biomass;            // Total biomass
    std::vector<glm::vec3> locations; // Where they live
};

struct EcosystemNiche {
    std::string name;
    float temperature;
    float pressure;
    float radiation;
    std::map<ElementType, float> availableElements;
    std::vector<Organism*> inhabitants;
    float carryingCapacity;
    float resourceCompetition;
};

class LifeEvolution {
public:
    LifeEvolution();
    
    // Core evolution mechanics
    void update(float deltaTime, const GlobalConditions& conditions);
    void introduceLife(const std::map<ElementType, float>& elements, 
                      const GlobalConditions& conditions);
    
    // Genetic operations
    Organism mutateOrganism(const Organism& parent, float mutationStrength);
    Organism hybridizeOrganisms(const Organism& parent1, const Organism& parent2);
    Gene createRandomGene(ElementType baseElement);
    Genome evolveGenome(const Genome& parent, const GlobalConditions& conditions);
    
    // Life chemistry determination
    LifeChemistry determineChemistry(const std::map<ElementType, float>& elements);
    GeneticCode selectGeneticCode(LifeChemistry chemistry, float complexity);
    Metabolism selectMetabolism(const GlobalConditions& conditions, 
                               const std::map<ElementType, float>& elements);
    
    // Environmental adaptation
    void adaptToEnvironment(Organism& organism, const EcosystemNiche& niche);
    float calculateFitness(const Organism& organism, const EcosystemNiche& niche);
    void naturalSelection(std::vector<Organism>& population, const EcosystemNiche& niche);
    
    // Ecosystem management
    void createEcosystemNiches(const std::map<ElementType, float>& globalElements);
    void updateEcosystems(float deltaTime);
    void simulateSpeciation();
    void simulateExtinction();
    
    // Intelligence and civilization
    void evolveIntelligence(Organism& organism);
    void developTechnology(Organism& organism);
    void emergeCivilization(std::vector<Organism>& species);
    
    // Getters
    const std::vector<Organism>& getAllOrganisms() const { return organisms; }
    const std::vector<EcosystemNiche>& getEcosystems() const { return ecosystems; }
    float getTotalBiomass() const;
    int getSpeciesCount() const { return static_cast<int>(organisms.size()); }
    float getAverageComplexity() const;
    std::string getEvolutionStatus() const;
    
    // Life events
    void triggerMassExtinction(float severity);
    void introduceAlienLife(LifeChemistry chemistry);
    void simulateEvolutionaryLeap();
    void createSymbiosis(Organism& org1, Organism& org2);

private:
    std::vector<Organism> organisms;
    std::vector<EcosystemNiche> ecosystems;
    std::mt19937 rng;
    
    float evolutionSpeed;
    float mutationPressure;
    float environmentalStress;
    float timeScale;
    
    // Evolution helpers
    float calculateMutationRate(const GlobalConditions& conditions);
    void applyEvolutionaryPressure(Organism& organism, float pressure);
    void handleResourceCompetition();
    void simulateGeneFlow();
    void trackEvolutionaryHistory();
    
    // Chemistry-specific evolution
    void evolveCarbonLife(Organism& organism, const GlobalConditions& conditions);
    void evolveSiliconLife(Organism& organism, const GlobalConditions& conditions);
    void evolveCrystalLife(Organism& organism, const GlobalConditions& conditions);
    void evolveEnergyLife(Organism& organism, const GlobalConditions& conditions);
    void evolveMetalLife(Organism& organism, const GlobalConditions& conditions);
    
    // Visualization helpers
    glm::vec3 getChemistryColor(LifeChemistry chemistry);
    float getComplexitySize(LifeComplexity complexity);
    std::string getOrganismDescription(const Organism& organism);
}; 