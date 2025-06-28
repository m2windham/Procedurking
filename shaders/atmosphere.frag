#version 330 core
out vec4 FragColor;

in vec3 worldPos;
in vec3 viewDir;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform float atmosphereRadius;
uniform float planetRadius;

// Rayleigh scattering coefficients
const vec3 betaR = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float scaleHeight = 0.25;

float rayleighPhase(float cosTheta) {
    return (3.0 / (16.0 * 3.14159)) * (1.0 + cosTheta * cosTheta);
}

vec2 rayIntersectSphere(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius) {
    vec3 oc = rayOrigin - sphereCenter;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0.0) {
        return vec2(-1.0, -1.0);
    }
    
    float sqrt_discriminant = sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0 * a);
    float t2 = (-b + sqrt_discriminant) / (2.0 * a);
    
    return vec2(t1, t2);
}

void main()
{
    vec3 rayOrigin = viewPos;
    vec3 rayDir = normalize(worldPos - viewPos);
    
    // Intersect with atmosphere
    vec2 atmosphereIntersect = rayIntersectSphere(rayOrigin, rayDir, vec3(0.0), atmosphereRadius);
    
    if (atmosphereIntersect.x < 0.0) {
        discard;
    }
    
    // Intersect with planet
    vec2 planetIntersect = rayIntersectSphere(rayOrigin, rayDir, vec3(0.0), planetRadius);
    
    float rayStart = max(atmosphereIntersect.x, 0.0);
    float rayEnd = atmosphereIntersect.y;
    
    // If we hit the planet, stop there
    if (planetIntersect.x > 0.0) {
        rayEnd = min(rayEnd, planetIntersect.x);
    }
    
    // Sample atmosphere
    int numSamples = 16;
    float segmentLength = (rayEnd - rayStart) / float(numSamples);
    
    vec3 scatteredLight = vec3(0.0);
    
    for (int i = 0; i < numSamples; i++) {
        float t = rayStart + (float(i) + 0.5) * segmentLength;
        vec3 samplePoint = rayOrigin + t * rayDir;
        float height = length(samplePoint) - planetRadius;
        
        // Calculate optical depth
        float density = exp(-height / scaleHeight);
        
        // Rayleigh scattering
        float cosTheta = dot(rayDir, lightDir);
        vec3 rayleigh = betaR * density * rayleighPhase(cosTheta);
        
        scatteredLight += rayleigh * segmentLength;
    }
    
    // Apply exposure and gamma correction
    vec3 color = 1.0 - exp(-scatteredLight * 20.0);
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, length(scatteredLight) * 0.1);
} 