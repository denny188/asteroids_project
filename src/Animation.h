#ifndef ANIMATION_H
#define ANIMATION_H

#include <SFML/Graphics.hpp>
#include <vector>

class Animation {
public:
    float frame, speed;
    sf::Sprite sprite;
    std::vector<sf::IntRect> frames;
    bool isPlaying; // Added to control playback
    bool loop;      // Added for looping control

    Animation();
    Animation(sf::Texture &t, int x, int y, int w, int h, int count, float speed, bool loopAnimation = true);

    void update(float dt); // Update based on delta time
    bool isFinished() const; // Check if non-looping animation finished
    void play();
    void pause();
    void reset();
};

#endif // ANIMATION_H