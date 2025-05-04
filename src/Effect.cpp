#include "Effect.h"

Effect::Effect() {
    type = Type::Effect;
    name = "effect";
    life = true; // Starts alive
    R = 0; // Effects typically don't collide
    velocity = sf::Vector2f(0,0); // Effects usually stay put
}

// Simplified settings for effects
void Effect::settings(Animation &a, sf::Vector2f startPos) {
    anim = a; // Use the passed animation directly
    pos = startPos;
    angle = 0; // No rotation usually needed
    R = 0;
    life = true;
    anim.reset(); // Start animation from beginning
    anim.play();
     name = "explosion"; // Or set based on animation type later
     this->type = Type::Effect;
}


void Effect::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;

    anim.update(dt);

    // Effect dies when its animation finishes (if not looping)
    if (anim.isFinished()) {
        life = false;
    }
}