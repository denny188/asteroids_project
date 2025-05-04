#include "PowerUp.h"
#include "ResourceManager.h"
#include <iostream>
#include <cmath>

PowerUp::PowerUp(PowerUpType type) : /* constructor logic same as before */
    isPowerDown(false), duration(5.0f), existenceTimer(10.0f) {
    this->type = Entity::Type::PowerUp; itemType.upType = type; name = "powerup_";
    switch (type) { /* name setting same as before */
        case PowerUpType::Shield:  name += "shield"; duration=8.0f; break; // Shield lasts longer
        case PowerUpType::Weapon:  name += "weapon"; duration=10.0f; break; // Weapon upgrade lasts longer
        case PowerUpType::Speed:   name += "speed"; duration=7.0f; break;
        case PowerUpType::ExtraLife: name += "extralife"; duration = 0; break;
    }
}

PowerUp::PowerUp(PowerDownType type) : /* constructor logic same as before */
    isPowerDown(true), duration(8.0f), existenceTimer(10.0f) {
    this->type = Entity::Type::PowerDown; itemType.downType = type; name = "powerdown_";
    switch (type) { /* name setting same as before */
        case PowerDownType::Slow: name += "slow"; break;
        case PowerDownType::ReverseControls: name += "reverse"; break;
        case PowerDownType::WeakerWeapon: name += "weaker"; break;
    }
}

void PowerUp::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    Animation actualAnim = a;
    float actualRadius = radius;
    std::string textureName = "";
    int frameW = 32, frameH = 32, frameCount = 1; // Default for single frame powerups
    float animSpeed = 0.f;
    bool loopAnim = true; // Most powerups might pulsate or rotate

     try {
        if (!isPowerDown) {
            switch (itemType.upType) {
                case PowerUpType::Shield:
                    textureName = "shield_powerup.png";
                    frameW = 556; frameH = 556; frameCount = 1; // Use full texture size
                    actualRadius = 15.f; // Collision radius
                    animSpeed = 0.1f; // Set a speed if you want it to rotate in Animation class
                    break;
                case PowerUpType::Weapon:
                    textureName = "weapon_powerup.png";
                    frameW = 256; frameH = 256; frameCount = 1;
                    actualRadius = 12.f;
                    break;
                case PowerUpType::Speed:
                     textureName = "speed_powerup.png";
                     frameW = 233; frameH = 134; frameCount = 1;
                     actualRadius = 12.f;
                     break;
                 // case PowerUpType::ExtraLife: textureName = "extralife_powerup.png"; break; // Add texture
            }
        } else {
             // Power-downs currently handled by HazardMeteor, might not need specific items
             // If you add collectible power-downs:
             // switch (itemType.downType) {
             //    case PowerDownType::Slow: textureName = "some_slow_icon.png"; break;
             //}
             std::cerr << "Warning: Trying to create collectible PowerDown - currently handled by HazardMeteor." << std::endl;
             life = false; // Don't create this entity for now
             return;
        }

        if (!textureName.empty()) {
            actualAnim = Animation(ResourceManager::getInstance().getTexture(textureName), 0, 0, frameW, frameH, frameCount, animSpeed, loopAnim);
            // Adjust scale if needed for visual size vs collision radius
            float visualScale = actualRadius * 2.0f / std::max(frameW, frameH); // Scale to roughly match radius visually
            actualAnim.sprite.setScale(visualScale, visualScale);
        } else if (!isPowerDown && itemType.upType == PowerUpType::ExtraLife) {
            // Handle case where ExtraLife texture might be missing, use fallback
             std::cerr << "Warning: Extra life texture missing, using fallback." << std::endl;
        }

    } catch (const std::runtime_error& e) {
         std::cerr << "Error setting powerup animation (" << name << "): " << e.what() << std::endl;
         actualAnim = a; // Keep fallback
         actualRadius = radius;
    }

    Entity::settings(actualAnim, startPos, startAngle, actualRadius);
    existenceTimer = 10.0f;
    life = true;
    this->type = isPowerDown ? Entity::Type::PowerDown : Entity::Type::PowerUp;
}

void PowerUp::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;
    existenceTimer -= dt;
    if (existenceTimer <= 0) { life = false; return; }

    // Optional: Add rotation or bobbing effect
    angle += 30.f * dt; // Simple rotation
    pos.y += std::sin(existenceTimer * 2.0f) * 0.5f; // Gentle bobbing

    anim.update(dt); // Update animation (might just be static or rotating)
}

// Getters same as before
bool PowerUp::getIsPowerDown() const { return isPowerDown; }
PowerUp::PowerUpType PowerUp::getPowerUpType() const { /* ... */ return isPowerDown ? PowerUpType::Shield : itemType.upType; }
PowerUp::PowerDownType PowerUp::getPowerDownType() const { /* ... */ return !isPowerDown ? PowerDownType::Slow : itemType.downType; }