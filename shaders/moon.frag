#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in float elevation;

uniform vec3 viewPos;
uniform float maxElevation;
uniform vec3 lightDir;

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
    
    // Lighting setup - moonlight is dimmer
    vec3 lightColor = vec3(0.9, 0.9, 0.85);
    vec3 ambientColor = vec3(0.1, 0.1, 0.15);
    
    // Calculate lighting
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Surface detail noise
    float surfaceNoise = fbm(FragPos * 80.0);
    float microDetail = fbm(FragPos * 300.0) * 0.2;
    
    // Normalize elevation
    float normalizedElevation = elevation / maxElevation;
    
    vec3 moonColor;
    
    // Moon surface materials - various shades of gray and brown
    if (elevation < -0.1) {
        // Deep craters - very dark
        moonColor = vec3(0.08, 0.08, 0.1);
    } else if (elevation < -0.05) {
        // Crater floors - dark gray
        moonColor = vec3(0.12, 0.12, 0.14);
    } else if (elevation < 0.0) {
        // Low areas - medium gray
        moonColor = vec3(0.18, 0.18, 0.2);
    } else if (elevation < 0.1) {
        // Plains - light gray
        vec3 regolith = vec3(0.25, 0.25, 0.27);
        vec3 dust = vec3(0.22, 0.20, 0.18);
        moonColor = mix(regolith, dust, surfaceNoise);
    } else if (elevation < 0.2) {
        // Crater rims and hills - brighter
        vec3 brightRegolith = vec3(0.35, 0.33, 0.30);
        vec3 rockOutcrop = vec3(0.28, 0.26, 0.24);
        moonColor = mix(brightRegolith, rockOutcrop, surfaceNoise * 0.6);
    } else {
        // High peaks - very bright (fresh crater material)
        vec3 freshMaterial = vec3(0.45, 0.42, 0.38);
        vec3 weatheredRock = vec3(0.35, 0.32, 0.28);
        moonColor = mix(freshMaterial, weatheredRock, surfaceNoise * 0.4);
    }
    
    // Apply surface detail variation
    moonColor *= (0.85 + microDetail * 0.3);
    
    // Lunar lighting - no atmosphere, sharp shadows
    vec3 ambient = ambientColor * 0.15;
    vec3 diffuse = lightColor * diff * 0.85;
    
    // No specular highlights - moon surface is matte
    
    vec3 finalColor = moonColor * (ambient + diffuse);
    
    // No atmospheric perspective - space is clear
    // Just apply slight distance darkening for depth
    float distance = length(viewPos - FragPos);
    float distanceFactor = 1.0 / (1.0 + distance * 0.001);
    finalColor *= distanceFactor;
    
    // Enhance contrast for lunar appearance
    finalColor = pow(finalColor, vec3(0.9));
    
    FragColor = vec4(finalColor, 1.0);
} 