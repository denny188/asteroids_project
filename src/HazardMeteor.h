#ifndef HAZARDMETEOR_H
#define HAZARDMETEOR_H

#include "Entity.h"

class HazardMeteor : public Entity {
public:
    HazardMeteor();

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 20.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;
    // No onCollision needed here, handled in Game::checkCollisions
};

#endif // HAZARDMETEOR_H