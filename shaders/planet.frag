#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float elevation;

uniform vec3 viewPos;
uniform float maxElevation;
uniform float oceanLevel;
uniform float time;
uniform float solarEnergy;   // 0.5 to 2.0 - affects temperature/ice/vegetation
uniform float volcanism;     // 0.1 to 2.0 - affects lava/volcanic activity
uniform float timeSpeed;     // Time acceleration factor for weather effects

// Simple noise function for surface detail
float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 3; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Lighting setup - affected by solar energy
    vec3 lightDir = normalize(vec3(0.3, 0.8, 0.5));
    vec3 lightColor = vec3(1.0, 0.98, 0.9) * solarEnergy;
    vec3 ambientColor = vec3(0.4, 0.5, 0.7) * (0.5 + solarEnergy * 0.5);
    
    // Calculate lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    // Surface detail with erosion effects
    float surfaceNoise = fbm(FragPos * 100.0);
    float slope = 1.0 - abs(dot(norm, normalize(FragPos)));
    
    // Weather and erosion effects (accelerated by time speed)
    float weatherTime = time * timeSpeed * 0.1;
    float erosion = fbm(FragPos * 30.0 + weatherTime * 0.05) * 0.3;
    float weathering = sin(weatherTime + FragPos.x * 10.0) * sin(weatherTime + FragPos.z * 10.0) * 0.1;
    
    // ENHANCED volcanic activity effects
    float volcanicNoise = fbm(FragPos * 30.0 + time * 0.2);
    float volcanicNoise2 = fbm(FragPos * 80.0 + time * 0.3);
    bool isVolcanic = volcanism > 0.8 && volcanicNoise > 0.6 && elevation > 0.05; // Much easier to trigger
    bool isLava = volcanism > 1.2 && volcanicNoise > 0.8 && volcanicNoise2 > 0.7; // Glowing lava
    
    // DRAMATIC temperature effects from solar energy
    float snowLine = mix(0.15, 0.6, clamp((solarEnergy - 0.5) / 1.5, 0.0, 1.0)); // More extreme ice ages/hot periods
    float vegetationThreshold = mix(0.05, 0.25, clamp((solarEnergy - 0.5) / 1.5, 0.0, 1.0)); // Vegetation zones shift dramatically
    
    // EXTREME climate effects - much more dramatic
    float iceAge = 1.0 - clamp((solarEnergy - 0.1) / 0.9, 0.0, 1.0); // Strong when solar < 1.0
    float greenhouse = clamp((solarEnergy - 0.9) / 0.9, 0.0, 1.0); // Strong when solar > 0.9
    
    // Make effects much more extreme
    iceAge = pow(iceAge, 0.5); // More dramatic curve
    greenhouse = pow(greenhouse, 0.5);
    
    // ENHANCED weather patterns based on climate
    float stormIntensity = mix(0.3, 1.5, greenhouse) * mix(1.2, 0.4, iceAge); // More storms in greenhouse, fewer in ice age
    float cloudCover = mix(0.2, 0.8, stormIntensity);
    
    float cloudShadow = sin(FragPos.x * 5.0 + weatherTime * 2.0) * sin(FragPos.z * 5.0 + weatherTime * 1.5) * cloudCover;
    float rainEffect = max(0.0, sin(weatherTime * 3.0 + FragPos.y * 20.0)) * stormIntensity;
    
    // Blizzard effects during ice ages
    float blizzard = iceAge * max(0.0, sin(weatherTime * 5.0 + FragPos.x * 15.0)) * 0.4;
    
    // Heat waves during greenhouse periods
    float heatShimmer = greenhouse * sin(weatherTime * 4.0 + FragPos.z * 12.0) * 0.3;
    
    vec3 terrainColor;
    float roughness = 0.8;
    
    // REALISTIC EARTH-LIKE BIOMES
    if (elevation < -0.05) {
        // Deep ocean - affected by climate
        vec3 normalOcean = vec3(0.02, 0.1, 0.3);
        vec3 frozenOcean = vec3(0.4, 0.5, 0.6); // Ice-covered ocean
        vec3 warmOcean = vec3(0.0, 0.2, 0.4); // Warmer, deeper blue
        
        terrainColor = mix(mix(normalOcean, frozenOcean, iceAge * 1.5), warmOcean, greenhouse * 1.2);
        terrainColor = clamp(terrainColor, 0.0, 1.0);
        
        // DRAMATIC ice coverage during ice ages
        if (iceAge > 0.3) {
            terrainColor = mix(terrainColor, vec3(0.9, 0.95, 1.0), iceAge * 1.2); // Thick ice
        }
        
        roughness = mix(0.1, 0.8, iceAge * 0.8); // Ice is rougher
    } else if (elevation < -0.01) {
        // Shallow ocean - medium blue with climate effects
        vec3 normalShallow = vec3(0.1, 0.3, 0.6);
        vec3 frozenShallow = vec3(0.5, 0.6, 0.7);
        vec3 tropicalShallow = vec3(0.0, 0.4, 0.7); // Tropical blue
        
        terrainColor = mix(mix(normalShallow, frozenShallow, iceAge * 1.8), tropicalShallow, greenhouse * 1.5);
        terrainColor = clamp(terrainColor, 0.0, 1.0);
        
        // Enhanced wave effects in warm climates, reduced in ice ages
        float waveStrength = mix(0.02, 0.05, greenhouse) * mix(1.0, 0.2, iceAge);
        float wave = sin(FragPos.x * 20.0 + time * 2.0) * sin(FragPos.z * 20.0 + time * 1.5) * waveStrength;
        terrainColor += vec3(wave * 0.5, wave * 0.3, wave);
        
        // DRAMATIC ice coverage on shallow water
        if (iceAge > 0.2) {
            terrainColor = mix(terrainColor, vec3(0.85, 0.9, 0.95), iceAge * 1.0);
        }
        
        roughness = mix(0.1, 0.7, iceAge * 0.6);
    } else if (elevation < 0.0) {
        // Very shallow water/coastal - light blue with weather effects
        terrainColor = vec3(0.3, 0.5, 0.8);
        terrainColor += rainEffect * vec3(0.1, 0.1, 0.2); // Rain darkening
        roughness = 0.2;
    } else if (elevation < 0.01) {
        // Beach/shore - sand color with erosion
        terrainColor = vec3(0.8, 0.7, 0.5);
        terrainColor += surfaceNoise * 0.1;
        terrainColor += erosion * vec3(0.1, 0.05, 0.0); // Sand movement
        roughness = 0.7;
    } else if (elevation < 0.03) {
        // Coastal plains - affected by climate
        vec3 baseColor = vec3(0.4, 0.6, 0.3);
        
        // EXTREME climate effects
        vec3 frozenColor = vec3(0.8, 0.9, 1.0); // Much whiter for ice
        vec3 tropicalColor = vec3(0.1, 0.8, 0.2); // Much greener for tropical
        
        // More dramatic mixing
        terrainColor = mix(mix(baseColor, frozenColor, iceAge * 1.5), tropicalColor, greenhouse * 1.5);
        terrainColor = clamp(terrainColor, 0.0, 1.0);
        terrainColor += surfaceNoise * 0.1;
        terrainColor += blizzard * vec3(0.3, 0.3, 0.4); // Blizzard whitening
        roughness = 0.8;
    } else if (elevation < 0.08) {
        // Lowlands/grasslands - DRAMATICALLY affected by climate
        vec3 baseGrass = vec3(0.3, 0.5, 0.2);
        vec3 droughtGrass = vec3(0.6, 0.4, 0.1); // Brown, dried out
        vec3 lushGrass = vec3(0.1, 0.6, 0.2); // Rich green
        vec3 frozenGrass = vec3(0.5, 0.6, 0.7); // Frozen tundra
        
        // EXTREME climate effects on grasslands
        terrainColor = mix(mix(baseGrass, frozenGrass, iceAge * 2.0), lushGrass, greenhouse * 2.0);
        terrainColor = clamp(terrainColor, 0.0, 1.0);
        
        // Seasonal effects
        float seasonalEffect = sin(weatherTime * 0.5) * 0.2;
        terrainColor *= (1.0 + seasonalEffect);
        
        // Drought effects in extreme heat
        if (greenhouse > 0.7 && surfaceNoise > 0.6) {
            terrainColor = mix(terrainColor, droughtGrass, 0.5);
        }
        
        // Add some brown patches with erosion
        if (surfaceNoise > 0.7) {
            terrainColor = mix(terrainColor, vec3(0.4, 0.3, 0.2), 0.3);
        }
        
        // Weather effects
        terrainColor += cloudShadow * vec3(-0.1, -0.1, -0.1); // Cloud shadows
        terrainColor += rainEffect * vec3(0.0, 0.1, 0.0); // Rain makes grass greener
        terrainColor += blizzard * vec3(0.4, 0.4, 0.5); // Blizzard effects
        terrainColor += heatShimmer * vec3(0.1, 0.05, 0.0); // Heat effects
        terrainColor += erosion * vec3(0.05, 0.02, 0.0); // Soil exposure
        
        roughness = 0.9;
    } else if (elevation < vegetationThreshold) {
        // Hills/forests - DRAMATICALLY affected by climate
        vec3 healthyForest = vec3(0.2, 0.4, 0.15);
        vec3 deadForest = vec3(0.4, 0.3, 0.2); // Dead/dying trees
        vec3 tropicalForest = vec3(0.1, 0.5, 0.2); // Lush tropical
        vec3 borealForest = vec3(0.3, 0.4, 0.3); // Cold climate forest
        
        // EXTREME forest climate effects
        terrainColor = mix(mix(healthyForest, borealForest, iceAge * 2.0), tropicalForest, greenhouse * 2.0);
        terrainColor = clamp(terrainColor, 0.0, 1.0);
        
        // Die-off in extreme conditions
        if (solarEnergy < 0.6 || solarEnergy > 1.6) {
            terrainColor = mix(terrainColor, deadForest, 0.4);
        }
        
        // Forest variation
        terrainColor += surfaceNoise * 0.05;
        terrainColor += blizzard * vec3(0.3, 0.3, 0.4); // Snow on trees
        terrainColor += heatShimmer * vec3(0.1, -0.05, -0.1); // Heat stress
        roughness = 0.95;
    } else if (elevation < 0.25) {
        // Mountains - brown/gray rock
        vec3 rock = vec3(0.4, 0.3, 0.25);
        vec3 vegetation = vec3(0.3, 0.4, 0.2) * clamp(solarEnergy, 0.3, 1.5);
        
        // DRAMATIC volcanic override
        if (isVolcanic) {
            rock = vec3(0.8, 0.3, 0.1); // Bright volcanic red
            if (isLava) {
                rock = vec3(1.0, 0.6, 0.0) + sin(time * 10.0) * 0.3; // Pulsing bright lava
            }
        }
        
        // Mix rock and vegetation based on slope
        terrainColor = mix(vegetation, rock, slope);
        terrainColor += surfaceNoise * 0.05;
        roughness = 0.8;
    } else if (elevation < snowLine) {
        // High mountains - gray rock
        terrainColor = vec3(0.35, 0.35, 0.4);
        
        // DRAMATIC volcanic peaks
        if (isVolcanic) {
            terrainColor = vec3(0.7, 0.2, 0.1); // Bright volcanic rock
            if (isLava) {
                terrainColor = vec3(1.0, 0.5, 0.0) + sin(time * 8.0) * 0.4; // Bright pulsing lava
            }
        }
        
        // Weather staining
        terrainColor += surfaceNoise * 0.03;
        roughness = 0.7;
    } else {
        // Snow peaks - DRAMATICALLY affected by climate
        vec3 rockColor = vec3(0.35, 0.35, 0.4);
        vec3 snowColor = vec3(0.9, 0.95, 1.0);
        vec3 glacierColor = vec3(0.7, 0.8, 0.95);
        
        // Ice coverage based on climate
        float ice_coverage = mix(1.0, 0.1, greenhouse) * mix(1.0, 1.5, iceAge); // More ice in ice age, less in greenhouse
        ice_coverage = clamp(ice_coverage, 0.0, 1.0);
        
        // During ice ages, even lower elevations get covered
        if (iceAge > 0.5 && elevation > snowLine * 0.7) {
            ice_coverage = max(ice_coverage, iceAge);
        }
        
        terrainColor = mix(rockColor, mix(snowColor, glacierColor, iceAge), ice_coverage);
        
        // DRAMATIC volcanic snow peaks - no snow on active volcanoes
        if (isVolcanic) {
            terrainColor = vec3(0.6, 0.15, 0.05); // Hot volcanic rock
            if (isLava) {
                terrainColor = vec3(1.0, 0.4, 0.0) + sin(time * 12.0) * 0.5; // Very bright pulsing lava
            }
        }
        
        // Weather effects
        terrainColor += blizzard * vec3(0.2, 0.2, 0.3); // Extra snow during blizzards
        terrainColor += surfaceNoise * 0.02;
        roughness = 0.3;
    }
    
    // Lighting calculation
    vec3 ambient = ambientColor * 0.3;
    vec3 diffuse = lightColor * diff * 0.7;
    vec3 specular = vec3(0.0);
    
    // Water specular
    if (elevation < 0.0) {
        specular = lightColor * spec * 0.8;
    }
    // Snow specular
    else if (elevation > 0.35) {
        specular = lightColor * spec * 0.4;
    }
    
    vec3 finalColor = terrainColor * (ambient + diffuse) + specular;
    
    // Atmospheric perspective
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-distance * 0.003);
    vec3 fogColor = vec3(0.6, 0.7, 0.9);
    finalColor = mix(fogColor, finalColor, fogFactor);
    
    // Gamma correction
    finalColor = pow(finalColor, vec3(0.9));
    
    FragColor = vec4(finalColor, 1.0);
} 