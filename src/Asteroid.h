#ifndef ASTEROID_H
#define ASTEROID_H

#include "Entity.h"

class Asteroid : public Entity {
public:
    enum class Size { Large, Medium, Small };

    Size asteroidSize;
    int scoreValue;

    Asteroid(Size sz = Size::Large); // Constructor takes size

    void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 25.f) override;
    void update(float dt, const sf::Vector2u& windowSize) override;

    Size getSize() const;
};

#endif // ASTEROID_H