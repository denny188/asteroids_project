#ifndef BULLET_H
#define BULLET_H

#include "Entity.h"

class Bullet : public Entity {
public:
    enum class BulletType { Standard, Laser, Spread, Red }; // Added Red type

    float speed;
    float lifetime;
    float lifeTimer;
    BulletType bulletType;
    int damage; // How much damage this bullet does

    Bullet(BulletType type = BulletType::Standard);

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 5.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;
};

#endif // BULLET_H