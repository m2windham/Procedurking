#version 330 core
out vec4 FragColor;

in float brightness;

uniform float time;

void main()
{
    // Create circular star shape
    vec2 center = gl_PointCoord - vec2(0.5);
    float distance = length(center);
    
    if (distance > 0.5) {
        discard;
    }
    
    // Star color variation based on brightness
    vec3 starColor;
    if (brightness > 0.8) {
        // Bright blue-white stars
        starColor = vec3(0.9, 0.95, 1.0);
    } else if (brightness > 0.6) {
        // White stars
        starColor = vec3(1.0, 1.0, 0.95);
    } else if (brightness > 0.4) {
        // Yellow stars
        starColor = vec3(1.0, 0.9, 0.7);
    } else {
        // Red dwarf stars
        starColor = vec3(1.0, 0.7, 0.5);
    }
    
    // Twinkling effect
    float twinkle = sin(time * 2.0 + gl_FragCoord.x * 0.1 + gl_FragCoord.y * 0.1) * 0.1 + 0.9;
    
    // Radial falloff for glow
    float alpha = (1.0 - distance * 2.0) * brightness * twinkle;
    alpha = clamp(alpha, 0.0, 1.0);
    
    FragColor = vec4(starColor * alpha, alpha);
} 