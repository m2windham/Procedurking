#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aInstancePos;    // Ring particle position
layout (location = 3) in float aInstanceSize;  // Ring particle size
layout (location = 4) in float aInstanceAlpha; // Ring particle opacity

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out float particleAlpha;
out vec3 worldPos;

void main()
{
    // Scale the quad by instance size
    vec3 scaledPos = aPos * aInstanceSize;
    
    // Billboard the particle to face the camera
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    vec3 particlePos = aInstancePos + 
                       cameraRight * scaledPos.x + 
                       cameraUp * scaledPos.y;
    
    worldPos = vec3(model * vec4(particlePos, 1.0));
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    TexCoord = aTexCoord;
    particleAlpha = aInstanceAlpha;
} 