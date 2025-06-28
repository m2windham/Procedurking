#include "Camera.h"
#include <algorithm>
#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM),
      Mode(FLY), Velocity(0.0f), OnGround(false), GroundHeight(0.0f)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    
    if (Mode == FLY) {
        // Flying mode - free movement in all directions
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    } else {
        // Walking mode - movement along spherical surface
        glm::vec3 surfaceNormal = glm::normalize(Position); // Normal pointing away from planet center
        glm::vec3 forward = glm::normalize(glm::cross(Right, surfaceNormal));
        glm::vec3 right = glm::normalize(glm::cross(surfaceNormal, forward));
        
        if (direction == FORWARD)
            Position += forward * velocity;
        if (direction == BACKWARD)
            Position -= forward * velocity;
        if (direction == LEFT)
            Position -= right * velocity;
        if (direction == RIGHT)
            Position += right * velocity;
        if (direction == JUMP)
            Jump();
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Mode == WALK) {
            // More restricted pitch in walk mode
            if (Pitch > 60.0f)
                Pitch = 60.0f;
            if (Pitch < -60.0f)
                Pitch = -60.0f;
        } else {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::SetMode(Camera_Mode mode)
{
    Mode = mode;
    if (mode == WALK) {
        // Completely reset all velocity when switching to walk
        Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // If we're far from the planet when switching to walk mode, 
        // teleport to a reasonable height above the surface
        float distanceFromCenter = glm::length(Position);
        std::cout << "Switching to WALK mode, distance from center: " << distanceFromCenter << std::endl;
        
        if (distanceFromCenter > 10.0f || distanceFromCenter < 1.8f) {
            // Force a good starting position for isometric view
            Position = glm::vec3(0.0f, 0.0f, 2.8f); // Safe position above planet
            std::cout << "Reset to safe isometric position at distance: " << glm::length(Position) << std::endl;
        }
        
        // Orient camera to look outward from planet surface
        alignCameraToSurface();
        
        OnGround = false; // Let physics system determine if we're on ground
    }
}

void Camera::ToggleMode()
{
    Mode = (Mode == FLY) ? WALK : FLY;
    std::cout << "Switched to " << (Mode == FLY ? "FLY" : "WALK") << " mode" << std::endl;
    if (Mode == WALK) {
        // Completely reset all velocity when switching to walk
        Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // If we're far from the planet when switching to walk mode, 
        // teleport to a reasonable height above the surface
        float distanceFromCenter = glm::length(Position);
        std::cout << "Switching to WALK mode, distance from center: " << distanceFromCenter << std::endl;
        
        if (distanceFromCenter > 10.0f || distanceFromCenter < 1.8f) {
            // Force a good starting position for isometric view
            Position = glm::vec3(0.0f, 0.0f, 2.8f); // Safe position above planet
            std::cout << "Reset to safe isometric position at distance: " << glm::length(Position) << std::endl;
        }
        
        // Orient camera to look outward from planet surface
        alignCameraToSurface();
        
        OnGround = false; // Let physics system determine if we're on ground
    }
}

void Camera::Jump()
{
    if (Mode == WALK && OnGround) {
        // Jump away from planet center (radially outward)
        glm::vec3 jumpDirection = glm::normalize(Position);
        Velocity += jumpDirection * JUMP_VELOCITY;
        OnGround = false;
    }
}

void Camera::SetGroundHeight(float height)
{
    GroundHeight = height;
    
    // If we're in walk mode and significantly below ground, teleport to surface
    if (Mode == WALK) {
        float currentDistanceFromCenter = glm::length(Position);
        if (currentDistanceFromCenter < height - 0.001f) {
            // We're inside the planet - teleport to surface
            glm::vec3 direction = glm::normalize(Position);
            Position = direction * (height + 0.003f); // Tiny offset for tiny player
            Velocity.y = 0.0f;
            OnGround = true;
        }
    }
}

void Camera::UpdatePhysics(float deltaTime)
{
    if (Mode == WALK) {
        // Check if we're in isometric mode (high altitude)
        float distanceFromCenter = glm::length(Position);
        bool isIsometric = distanceFromCenter > 2.6f; // Above 2.6 = isometric mode (planet radius 2.0 + altitude 0.8)
        
        if (!isIsometric) {
            // Apply gravity towards planet center (but much gentler)
            glm::vec3 planetCenter = glm::vec3(0.0f);
            glm::vec3 gravityDirection = glm::normalize(planetCenter - Position);
            
            // Apply reduced gravity in the direction of planet center
            float gravityStrength = 1.0f; // Very gentle for tiny player
            Velocity += gravityDirection * gravityStrength * deltaTime;
            
            // Limit velocity magnitude to prevent launching
            float maxVelocity = 2.0f; // Much lower for tiny player
            if (glm::length(Velocity) > maxVelocity) {
                Velocity = glm::normalize(Velocity) * maxVelocity;
            }
            
            // Update position with velocity
            glm::vec3 newPosition = Position + Velocity * deltaTime;
            
            // Check ground collision using distance from center
            if (GroundHeight > 0.0f) {
                float newDistanceFromCenter = glm::length(newPosition);
                float playerHeight = 0.003f; // TINY player height above surface
                float minAllowedDistance = GroundHeight + playerHeight;
                
                if (newDistanceFromCenter <= minAllowedDistance) {
                    // We're at or below ground level - project to surface
                    glm::vec3 direction = glm::normalize(newPosition);
                    newPosition = direction * minAllowedDistance;
                    
                    // Remove velocity component toward planet center (damping)
                    glm::vec3 surfaceNormal = direction;
                    float velocityTowardSurface = glm::dot(Velocity, -surfaceNormal);
                    if (velocityTowardSurface > 0) {
                        Velocity -= velocityTowardSurface * (-surfaceNormal);
                    }
                    
                    // Apply friction when on ground
                    Velocity *= 0.95f;
                    
                    OnGround = true;
                } else {
                    OnGround = false;
                }
            }
            
            Position = newPosition;
        }
        // If isometric, no physics - free floating god view
    }
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::applyGravity(float deltaTime)
{
    if (!OnGround) {
        Velocity.y += GRAVITY * deltaTime;
        Position.y += Velocity.y * deltaTime;
    }
}

void Camera::handleGroundCollision()
{
    const float groundOffset = 0.1f; // Small offset above ground
    
    if (Position.y <= GroundHeight + groundOffset) {
        Position.y = GroundHeight + groundOffset;
        if (Velocity.y < 0.0f) {
            Velocity.y = 0.0f;
        }
        OnGround = true;
    } else {
        OnGround = false;
    }
}

void Camera::alignCameraToSurface()
{
    std::cout << "Setting up isometric view at 5000ft altitude" << std::endl;
    
    // Simple isometric setup: position camera above the planet looking down
    // Use a fixed position that's always above the planet
    float altitude = 0.8f; // Higher altitude for better view
    
    // Position camera above the current location (or a good default spot)
    glm::vec3 currentDirection = glm::normalize(Position);
    Position = currentDirection * (2.0f + altitude); // Planet radius + altitude
    
    // Look straight down towards planet center with slight angle
    glm::vec3 planetCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lookDirection = glm::normalize(planetCenter - Position);
    
    // Set camera to look down at the planet
    Front = lookDirection;
    
    // Create proper right and up vectors
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    Right = glm::normalize(glm::cross(Front, worldUp));
    Up = glm::normalize(glm::cross(Right, Front));
    
    // Calculate angles
    Yaw = glm::degrees(atan2(Front.z, Front.x));
    Pitch = glm::degrees(asin(-Front.y));
    
    // Ensure we're looking down
    if (Pitch > -30.0f) {
        Pitch = -45.0f; // Force a good downward angle
    }
    
    // Update camera vectors with the corrected angles
    updateCameraVectors();
    
    std::cout << "Isometric position: " << Position.x << ", " << Position.y << ", " << Position.z << std::endl;
    std::cout << "Looking down at: " << Pitch << " degrees" << std::endl;
} 