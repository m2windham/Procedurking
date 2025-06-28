#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in float particleAlpha;
in vec3 worldPos;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform float time;

// Simple noise function for particle variation
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Create circular particle shape
    vec2 center = TexCoord - vec2(0.5);
    float distance = length(center);
    
    if (distance > 0.5) {
        discard;
    }
    
    // Distance from planet center
    float distanceFromPlanet = length(worldPos);
    
    // Ring particle material variation
    float particleNoise = noise(worldPos.xy * 100.0);
    vec3 particleColor;
    
    if (particleNoise > 0.7) {
        // Ice particles - bright and reflective
        particleColor = vec3(0.9, 0.95, 1.0);
    } else if (particleNoise > 0.4) {
        // Rocky particles - darker
        particleColor = vec3(0.4, 0.35, 0.3);
    } else {
        // Dusty particles - brown/gray
        particleColor = vec3(0.3, 0.25, 0.2);
    }
    
    // Lighting calculation
    vec3 norm = normalize(vec3(0.0, 1.0, 0.0)); // Rings are mostly flat
    float lightFactor = max(dot(norm, lightDir), 0.0) * 0.8 + 0.2;
    
    // Add some scattering based on distance
    float scattering = 1.0 / (1.0 + distanceFromPlanet * 0.5);
    lightFactor *= scattering;
    
    // Ring density variation based on distance (scaled up 2x)
    float ringDensity = 1.0;
    if (distanceFromPlanet < 3.6f) { // Was 1.8f, now 3.6f
        // Inner ring gap (Roche limit effect)
        ringDensity *= 0.3;
    } else if (distanceFromPlanet > 7.0f) { // Was 3.5f, now 7.0f
        // Outer ring fade
        ringDensity *= max(0.0, 1.0 - (distanceFromPlanet - 7.0f) * 0.5); // Was 3.5f, now 7.0f
    }
    
    // Particle edge softening
    float edgeSoftness = 1.0 - smoothstep(0.3, 0.5, distance);
    
    // Final color and alpha
    vec3 finalColor = particleColor * lightFactor;
    float finalAlpha = particleAlpha * ringDensity * edgeSoftness * 0.4;
    
    // Add subtle shimmer effect
    float shimmer = sin(time * 3.0 + worldPos.x * 10.0 + worldPos.z * 10.0) * 0.1 + 0.9;
    finalColor *= shimmer;
    
    FragColor = vec4(finalColor, finalAlpha);
} 