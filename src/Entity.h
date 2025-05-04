#ifndef ENTITY_H
#define ENTITY_H

#include "Animation.h"
#include <SFML/Graphics.hpp>
#include <string>

class Game;

class Entity {
public:
    // Expanded types
    enum class Type { Generic, Player, Asteroid, Bullet, PowerUp, PowerDown, Effect, Boss, HazardMeteor };

    sf::Vector2f pos;
    sf::Vector2f velocity;
    float R;
    float angle;
    bool life;
    std::string name;
    Animation anim;
    Type type;

    Entity();
    virtual ~Entity() = default;

    virtual void settings(Animation &a, sf::Vector2f startPos, float startAngle = 0.f, float radius = 1.f);
    virtual void update(float dt, const sf::Vector2u& windowSize) = 0;
    virtual void draw(sf::RenderTarget &target);
    virtual void onCollision(Entity* other) {};
};

#endif // ENTITY_H