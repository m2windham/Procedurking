#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    JUMP
};

enum Camera_Mode {
    FLY,
    WALK
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.0f;        // Good speed for isometric god view
const float SENSITIVITY = 0.05f; // More precise mouse control for small scale
const float ZOOM = 45.0f;
const float GRAVITY = -9.81f;
const float JUMP_VELOCITY = 1.0f; // Much smaller jump for tiny player

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    
    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    Camera_Mode Mode;
    glm::vec3 Velocity;
    bool OnGround;
    float GroundHeight;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    glm::mat4 GetViewMatrix();

    void ProcessKeyboard(Camera_Movement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void SetMode(Camera_Mode mode);
    void ToggleMode();
    void Jump();
    
    // Terrain interaction
    void SetGroundHeight(float height);
    void UpdatePhysics(float deltaTime);

private:
    void updateCameraVectors();
    void applyGravity(float deltaTime);
    void handleGroundCollision();
    void alignCameraToSurface();
}; 