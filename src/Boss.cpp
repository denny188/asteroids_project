#include "Boss.h"
#include "ResourceManager.h"
#include <cmath>
#include <iostream> // For debug

const float BOSS_DEGTORAD = 0.017453f;

Boss::Boss() :
    maxHealth(100), // Example health
    health(100),
    phaseTimer(0.f),
    currentPhase(1),
    shootTimer1(0.f), shootTimer2(0.f), shootTimer3(0.f),
    shootCooldown(1.5f) // Base cooldown
{
    type = Type::Boss;
    name = "boss1"; // Or determine based on level
    R = 100.f; // Large collision radius

    // Define relative firing points (adjust based on boss1.png sprite)
    // Assuming origin is center of the 230x336 sprite
    firePoint1 = sf::Vector2f(0, -150);     // Top middle
    firePoint2 = sf::Vector2f(-80, -100);   // Top left-ish
    firePoint3 = sf::Vector2f(80, -100);    // Top right-ish
}

void Boss::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    Animation actualAnim = a; // Fallback
    float actualRadius = radius;
    try {
        // TODO: Handle different boss types if needed
        // boss1.png: 230x336, 1 frame
        actualAnim = Animation(ResourceManager::getInstance().getTexture("boss1.png"), 0, 0, 230, 336, 1, 0, false); // Static image
        actualRadius = 100.f; // Adjust collision radius if needed
    } catch (const std::runtime_error& e) {
        std::cerr << "Error setting Boss animation: " << e.what() << std::endl;
        actualAnim = a;
        actualRadius = radius;
    }

    Entity::settings(actualAnim, startPos, startAngle, actualRadius);
    health = maxHealth; // Reset health on setting
    currentPhase = 1;
    phaseTimer = 0.f;
    shootTimer1 = shootCooldown; // Initial delay before firing
    shootTimer2 = shootCooldown * 1.2f;
    shootTimer3 = shootCooldown * 1.4f;
    name = "boss1";
    type = Type::Boss;
}

void Boss::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;

    updatePhase(dt);
    updateMovement(dt, windowSize);
    updateShooting(dt); // Game class will actually spawn bullets based on boss state

    anim.update(dt); // Update animation (might be static)
}

void Boss::updatePhase(float dt) {
    phaseTimer += dt;
    // Example phase transition (e.g., based on health or time)
    if (health <= maxHealth / 2 && currentPhase == 1) {
        currentPhase = 2;
        phaseTimer = 0.f;
        shootCooldown = 1.0f; // Faster shooting in phase 2
        std::cout << "Boss entering Phase 2!" << std::endl;
    }
     // Add more phase logic
}

void Boss::updateMovement(float dt, const sf::Vector2u& windowSize) {
    // Simple movement pattern (e.g., side to side)
    float speed = 50.0f; // Pixels per second
    float patrolWidth = windowSize.x * 0.6f;
    float centerX = windowSize.x / 2.f;
    float targetX = centerX + (patrolWidth / 2.f) * std::sin(phaseTimer * 0.5f); // Sin wave movement

    if (pos.x < targetX) {
        velocity.x = speed;
    } else if (pos.x > targetX) {
        velocity.x = -speed;
    } else {
        velocity.x = 0;
    }
    // Keep boss near the top
    pos.y = windowSize.y * 0.2f;

    pos.x += velocity.x * dt;
    pos.x = std::max(R, std::min(windowSize.x - R, pos.x)); // Clamp to screen bounds

    // Optional: Slight rotation?
    // angle = 5.f * std::sin(phaseTimer * 1.5f);
}

void Boss::updateShooting(float dt) {
     // Decrement timers (Game class checks these timers to spawn bullets)
     if (shootTimer1 > 0) shootTimer1 -= dt;
     if (shootTimer2 > 0) shootTimer2 -= dt;
     if (shootTimer3 > 0) shootTimer3 -= dt;

     // Logic based on phase
     if (currentPhase == 1) {
         phase1Logic(dt);
     } else if (currentPhase == 2) {
          phase2Logic(dt);
     }
}

void Boss::phase1Logic(float dt) {
    // Example: Fire from middle gun periodically
     // Timers decremented in updateShooting. Game::spawnBossBullet handles spawning.
}

void Boss::phase2Logic(float dt) {
     // Example: Fire from all guns more frequently
     // Timers decremented in updateShooting. Game::spawnBossBullet handles spawning.
}


void Boss::takeDamage(int amount) {
    if (!life) return;
    health -= amount;
    std::cout << "Boss health: " << health << "/" << maxHealth << std::endl;
    if (health <= 0) {
        health = 0;
        life = false; // Boss defeated
        std::cout << "Boss defeated!" << std::endl;
        // TODO: Trigger boss explosion sequence in Game class
    }
    // TODO: Add hit flash effect?
}

void Boss::onCollision(Entity* other) {
    // Handle collision specific to boss if needed (e.g., ramming damage?)
    if (!life) return;
    if (other && other->type == Entity::Type::Player) {
        // Ramming damage?
        // other->takeDamage(); // Assuming player has takeDamage
    }
}

// Helper to get world coordinates of fire points
sf::Vector2f Boss::getAbsoluteFirePos(const sf::Vector2f& relativePos) {
    float rad = (angle) * BOSS_DEGTORAD; // Boss's current angle in radians
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);
    // Rotate the relative point and add to boss's position
    return pos + sf::Vector2f(relativePos.x * cosA - relativePos.y * sinA,
                              relativePos.x * sinA + relativePos.y * cosA);
}