#include "Asteroid.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

Asteroid::Asteroid(Size sz) : asteroidSize(sz) {
    type = Type::Asteroid;
    name = "asteroid";

    switch (asteroidSize) {
        case Size::Large: scoreValue = 20; R = 25.f; break;
        case Size::Medium: scoreValue = 50; R = 15.f; break; // Adjusted default R
        case Size::Small: scoreValue = 100; R = 8.f; break;  // Adjusted default R
    }

    float angleRad = (rand() % 360) * 0.017453f;
    float speed = static_cast<float>(rand() % 3 + 2);
    velocity.x = std::cos(angleRad) * speed;
    velocity.y = std::sin(angleRad) * speed;
}

void Asteroid::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    Animation actualAnim = a;
    float actualRadius = radius;
    float animSpeed = 0.2f; // Default speed

     try {
        switch (asteroidSize) {
            case Size::Large:
                 actualAnim = Animation(ResourceManager::getInstance().getTexture("rock.png"), 0, 0, 64, 64, 16, 0.2f); // W=1024/16=64
                 actualRadius = 25.f;
                 animSpeed = 0.2f;
                break;
            case Size::Medium:
                 // rock_medium.png: 1152x96, 12 frames
                 actualAnim = Animation(ResourceManager::getInstance().getTexture("rock_medium.png"), 0, 0, 96, 96, 12, 0.25f); // W=1152/12=96
                 actualRadius = 15.f;
                 animSpeed = 0.25f;
                 break;
            case Size::Small:
                 // rock_small.png: 1024x64, 16 frames
                 actualAnim = Animation(ResourceManager::getInstance().getTexture("rock_small.png"), 0, 0, 64, 64, 16, 0.3f); // W=1024/16=64
                 actualRadius = 8.f;
                 animSpeed = 0.3f;
                 break;
        }
     } catch(const std::runtime_error& e) {
         std::cerr << "Error setting asteroid animation: " << e.what() << std::endl;
         actualAnim = a;
         actualRadius = radius; // Keep fallback
     }

    Entity::settings(actualAnim, startPos, startAngle, actualRadius);
    name = "asteroid";
    type = Type::Asteroid;
}


void Asteroid::update(float dt, const sf::Vector2u& windowSize) {
    // Same as before
    if (!life) return;
    pos += velocity * dt * 60.f;
    if (pos.x < -R) pos.x = windowSize.x + R;
    else if (pos.x > windowSize.x + R) pos.x = -R;
    if (pos.y < -R) pos.y = windowSize.y + R;
    else if (pos.y > windowSize.y + R) pos.y = -R;
    anim.update(dt);
}

Asteroid::Size Asteroid::getSize() const {
    return asteroidSize;
}