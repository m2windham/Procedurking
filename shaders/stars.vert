#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aBrightness;

uniform mat4 view;
uniform mat4 projection;

out float brightness;

void main()
{
    brightness = aBrightness;
    
    // Remove translation from view matrix for skybox effect
    mat4 skyboxView = mat4(mat3(view));
    
    gl_Position = projection * skyboxView * vec4(aPos, 1.0);
    gl_Position = gl_Position.xyww; // Ensure stars are always at max depth
    
    // Variable star size based on brightness
    gl_PointSize = brightness * 3.0 + 1.0;
} 