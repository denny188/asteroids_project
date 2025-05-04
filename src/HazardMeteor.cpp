#include "HazardMeteor.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

HazardMeteor::HazardMeteor() {
    type = Type::HazardMeteor;
    name = "hazard_meteor";
    R = 20.f; // Collision radius

    // Give it a slower, more predictable movement?
    float angleRad = (rand() % 360) * 0.017453f;
    float speed = static_cast<float>(rand() % 2 + 1); // Slow speed (1-2)
    velocity.x = std::cos(angleRad) * speed;
    velocity.y = std::sin(angleRad) * speed;
}

void HazardMeteor::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    Animation actualAnim = a;
    float actualRadius = radius;
    float animSpeed = 0.3f;

    try {
        // slow_powerdown.png: 1536x64, 24 frames
        int frameW = 1536 / 24; // 64
        int frameH = 64;
        actualAnim = Animation(ResourceManager::getInstance().getTexture("slow_powerdown.png"), 0, 0, frameW, frameH, 24, animSpeed, true); // Looping animation
        actualRadius = 20.f; // Set appropriate collision radius
    } catch (const std::runtime_error& e) {
        std::cerr << "Error setting HazardMeteor animation: " << e.what() << std::endl;
        actualAnim = a; // Fallback
        actualRadius = radius;
    }

    Entity::settings(actualAnim, startPos, startAngle, actualRadius);
    name = "hazard_meteor";
    type = Type::HazardMeteor;
}

void HazardMeteor::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;

    pos += velocity * dt * 60.f; // Apply velocity

    // Screen wrapping (like asteroids)
    if (pos.x < -R) pos.x = windowSize.x + R;
    else if (pos.x > windowSize.x + R) pos.x = -R;
    if (pos.y < -R) pos.y = windowSize.y + R;
    else if (pos.y > windowSize.y + R) pos.y = -R;

    anim.update(dt); // Update animation frame
}