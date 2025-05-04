#include "Entity.h"

const float DEGTORAD = 0.017453f;

Entity::Entity() : R(1.f), angle(0.f), life(true), type(Type::Generic) {
    // Default velocity and position are (0,0)
}

// Provide a default implementation or make it pure virtual in the header
void Entity::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    anim = a; // Animation copy (consider using pointers/references if animations are shared heavily)
    pos = startPos;
    angle = startAngle;
    R = radius;
    life = true; // Ensure entity starts alive
    anim.reset(); // Reset animation state
    anim.play();
}

void Entity::draw(sf::RenderTarget &target) {
    if (!life) return; // Don't draw dead entities

    anim.sprite.setPosition(pos);
    anim.sprite.setRotation(angle); // Offset often needed depending on sprite orientation
    target.draw(anim.sprite);

    // --- Debug Circle Drawing (Uncomment to visualize collision radius) ---
    /*
    sf::CircleShape circle(R);
    circle.setFillColor(sf::Color(255, 0, 0, 100)); // Semi-transparent red
    circle.setOrigin(R, R); // Center the origin
    circle.setPosition(pos);
    target.draw(circle);
    */
}

// update method is pure virtual, no implementation here.
// Subclasses MUST implement it.