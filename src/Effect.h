#ifndef EFFECT_H
#define EFFECT_H

#include "Entity.h"

class Effect : public Entity {
public:
    Effect();
    // Settings specific to effects (mainly animation)
    void settings(Animation &a, sf::Vector2f startPos);
    void update(float dt, const sf::Vector2u& windowSize) override;

};

#endif // EFFECT_H