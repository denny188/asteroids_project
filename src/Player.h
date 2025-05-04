#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Bullet.h" // Include Bullet for weapon type
#include "PowerUp.h" // Include PowerUp for power-down type

class Player : public Entity {
public:
    enum class ShipType { Standard, Fast, Heavy };

    bool thrust;
    float maxSpeed;
    float acceleration;
    float turnSpeed;
    float friction;
    float shootCooldown;
    float shootTimer;
    int score;
    int lives;
    ShipType currentShipType;
    Bullet::BulletType currentWeaponType; // Track current weapon

    // Power-up/down states
    bool shieldActive;
    float shieldTimer;
    float speedBoostTimer;
    float slowTimer; // Timer for slow effect
    bool reverseControlsActive; // Flag for reversed controls
    float reverseControlsTimer;
    bool weaponPowerUpActive; // Flag for weapon powerup visual
    float weaponPowerUpTimer;

    Player();

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 20.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;
    void reset();

    void takeDamage();
    void addScore(int points);
    void applyPowerUp(PowerUp* item); // Handles both up and down
    void shoot(); // Moved shoot logic trigger here

    void setShipType(ShipType type);
    // Override draw to handle power-up visuals
    void draw(sf::RenderTarget &target) override;

private:
    void handleInput(float dt);
    void applyMovement(float dt, const sf::Vector2u& windowSize);
    void updateEffects(float dt); // Renamed from updatePowerUps

    // Animations
    Animation anim_idle;
    Animation anim_thrust;
    // Textures for overlays (loaded once)
    sf::Texture* shieldTexturePtr;
    sf::Texture* weaponEffectTexturePtr;
    sf::Texture* speedEffectTexturePtr;
     // Sprite for overlays
    sf::Sprite shieldEffectSprite;
    sf::Sprite weaponEffectSprite;
    sf::Sprite speedEffectSprite;
};

#endif // PLAYER_H