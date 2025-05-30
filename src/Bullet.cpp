#include "Bullet.h"
#include "ResourceManager.h"
#include <cmath>
#include <iostream>

const float BULLET_DEGTORAD = 0.017453f;

Bullet::Bullet(BulletType type) :
    speed(10.0f),
    lifetime(1.5f),
    lifeTimer(0.f),
    bulletType(type),
    damage(1) // Default damage
{
    this->type = Type::Bullet;
    name = "bullet_"; // Base name

    switch (bulletType) {
        case BulletType::Standard:
            name += "standard";
            speed = 10.0f; damage = 1;
            break;
        case BulletType::Laser:
            name += "laser";
            speed = 18.0f; lifetime = 0.8f; damage = 1; // Laser: fast, short life?
            break;
        case BulletType::Spread:
             name += "spread";
             speed = 8.0f; lifetime = 1.0f; damage = 1;
             break;
        case BulletType::Red: // Added Red bullet type
             name += "red";
             speed = 12.0f; damage = 2; // Faster and more damage?
             break;
    }
}

void Bullet::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    Animation actualAnim = a;
    float actualRadius = radius;
    float animSpeed = 0.8f; // Default speed

     try {
         std::string textureFile;
         int frameW = 32, frameH = 64, frameCount = 16; // Defaults for blue/red
         switch(bulletType) {
            case BulletType::Standard:
                 textureFile = "fire_blue.png";
                 actualRadius = 5.f; animSpeed = 0.8f;
                 break;
            case BulletType::Laser:
                 textureFile = "fire_laser.png"; // Use the laser texture
                 frameW = 64; frameH = 64; frameCount = 18; // W=1152/18=64
                 actualRadius = 4.f; animSpeed = 1.2f; // Faster animation?
                 break;
            case BulletType::Spread:
                 textureFile = "fire_blue.png"; // Use standard look for spread
                 actualRadius = 4.f; animSpeed = 0.8f;
                 break;
             case BulletType::Red:
                  textureFile = "fire_red.png"; // Use the red texture
                  actualRadius = 5.f; animSpeed = 0.9f; // Slightly faster anim?
                  break;
         }
         actualAnim = Animation(ResourceManager::getInstance().getTexture(textureFile), 0, 0, frameW, frameH, frameCount, animSpeed, false); // Non-looping

     } catch (const std::runtime_error& e) {
        std::cerr << "Error setting bullet animation (" << name << "): " << e.what() << std::endl;
        actualAnim = a; // Fallback
        actualRadius = radius;
     }

    Entity::settings(actualAnim, startPos, startAngle, actualRadius);

    float angleRad = this->angle * BULLET_DEGTORAD; // Chuyển góc sang Radian
    velocity.x = std::sin(angleRad) * speed;      // Thành phần X theo sin
    velocity.y = -std::cos(angleRad) * speed;     // Thành phần Y theo -cos (vì Y hướng xuống)

    lifeTimer = 0;
    life = true;
    // Name and type already set in constructor
}

void Bullet::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;

    pos += velocity * dt * 60.f;
    lifeTimer += dt;

    if (lifeTimer >= lifetime) {
        life = false;
        return;
    }
     if (pos.x < -R || pos.x > windowSize.x + R || pos.y < -R || pos.y > windowSize.y + R) {
        life = false;
        return;
     }
    anim.update(dt);
}