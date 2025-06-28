#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldPos;
out vec3 viewDir;

void main()
{
    worldPos = vec3(model * vec4(aPos, 1.0));
    viewDir = normalize(worldPos);
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
} 