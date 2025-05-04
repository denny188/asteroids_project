#ifndef BOSS_H
#define BOSS_H

#include "Entity.h"
#include <vector> // For fire points

class Boss : public Entity {
public:
    int health;
    int maxHealth;
    float phaseTimer;
    int currentPhase;
    float shootTimer1, shootTimer2, shootTimer3; // Timers for different guns
    float shootCooldown;

    // Relative positions for firing points
    sf::Vector2f firePoint1, firePoint2, firePoint3;

    Boss();

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 100.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;
    void takeDamage(int amount);
    void onCollision(Entity* other) override;

    // Function to get absolute fire point positions
    sf::Vector2f getAbsoluteFirePos(const sf::Vector2f& relativePos);

private:
    void updatePhase(float dt);
    void updateMovement(float dt, const sf::Vector2u& windowSize);
    void updateShooting(float dt);
    void phase1Logic(float dt);
    void phase2Logic(float dt); // Add more phases if needed
};

#endif // BOSS_H