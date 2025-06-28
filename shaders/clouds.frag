#version 330 core
out vec4 FragColor;

in vec3 worldPos;
in vec3 cloudCoord;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform float time;

// Simple noise function
float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}

void main()
{
    vec3 cloudPos = cloudCoord * 3.0;
    
    // Multi-layer cloud generation
    float cloud1 = fbm(cloudPos);
    float cloud2 = fbm(cloudPos * 2.0 + vec3(time * 0.02));
    float cloud3 = fbm(cloudPos * 4.0 - vec3(time * 0.01));
    
    // Combine cloud layers
    float cloudDensity = cloud1 * 0.5 + cloud2 * 0.3 + cloud3 * 0.2;
    cloudDensity = smoothstep(0.4, 0.8, cloudDensity);
    
    // Cloud coverage variation
    float coverage = fbm(cloudPos * 0.5) * 0.8 + 0.2;
    cloudDensity *= coverage;
    
    // Lighting calculation
    vec3 norm = normalize(worldPos);
    float lightFactor = max(dot(norm, lightDir), 0.0);
    
    // Cloud color based on lighting
    vec3 cloudColor = mix(vec3(0.3, 0.3, 0.4), vec3(1.0, 0.98, 0.95), lightFactor);
    
    // Add some atmospheric scattering
    float viewAngle = dot(normalize(viewPos - worldPos), lightDir);
    float scattering = pow(max(viewAngle, 0.0), 2.0);
    cloudColor += vec3(0.2, 0.3, 0.5) * scattering * 0.3;
    
    // Final alpha based on density
    float alpha = cloudDensity * 0.6;
    
    // Fade clouds at edges
    float edgeFade = 1.0 - smoothstep(0.8, 1.0, length(cloudCoord));
    alpha *= edgeFade;
    
    FragColor = vec4(cloudColor, alpha);
} 