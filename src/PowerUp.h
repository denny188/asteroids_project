#ifndef POWERUP_H
#define POWERUP_H

#include "Entity.h"

class PowerUp : public Entity {
public:
    enum class PowerUpType { Shield, Weapon, Speed, ExtraLife /* Add more */ };
    enum class PowerDownType { Slow, ReverseControls, WeakerWeapon /* Add more */ };

    bool isPowerDown; // Flag to indicate if it's a negative effect
    union { // Use a union for type safety
        PowerUpType upType;
        PowerDownType downType;
    } itemType;

    float duration; // How long the effect lasts (if applicable)
    float existenceTimer; // How long the power-up stays on screen

    PowerUp(PowerUpType type); // Constructor for PowerUps
    PowerUp(PowerDownType type); // Constructor for PowerDowns

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 12.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;

    // Getters for type checking
    bool getIsPowerDown() const;
    PowerUpType getPowerUpType() const;     // Only valid if !isPowerDown
    PowerDownType getPowerDownType() const; // Only valid if isPowerDown
};

#endif // POWERUP_H