#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 worldPos;
out vec3 cloudCoord;

void main()
{
    worldPos = vec3(model * vec4(aPos, 1.0));
    
    // Create moving cloud coordinates
    cloudCoord = aPos + vec3(time * 0.01, 0.0, time * 0.005);
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
} 