#include "Animation.h"

Animation::Animation() : frame(0.f), speed(0.f), isPlaying(true), loop(true) {}

Animation::Animation(sf::Texture &t, int x, int y, int w, int h, int count, float Speed, bool loopAnimation)
    : frame(0.f), speed(Speed), isPlaying(true), loop(loopAnimation)
{
    frames.reserve(count);
    for (int i = 0; i < count; i++) {
        frames.emplace_back(x + i * w, y, w, h);
    }

    sprite.setTexture(t);
    sprite.setOrigin(static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f);
    if (!frames.empty()) {
        sprite.setTextureRect(frames[0]);
    } else {
        // Handle error or set a default rect if needed
    }
}

void Animation::update(float dt) {
    if (!isPlaying || speed == 0.f) return; // Don't update if paused or speed is zero

    frame += speed * dt * 60.f; // Multiply by 60 to keep speed consistent with original logic if dt is ~1/60

    int n = static_cast<int>(frames.size());
    if (n == 0) return; // No frames to animate

    if (loop) {
        while (frame >= n) frame -= n; // Loop around
    } else {
        if (frame >= n) {
            frame = static_cast<float>(n - 1); // Clamp to last frame
            isPlaying = false;           // Stop playing if not looping
        }
    }

    sprite.setTextureRect(frames[static_cast<int>(frame)]);
}

bool Animation::isFinished() const {
    return !loop && !isPlaying && !frames.empty() && static_cast<int>(frame) == frames.size() - 1;
}

void Animation::play() {
    isPlaying = true;
}

void Animation::pause() {
    isPlaying = false;
}

void Animation::reset() {
    frame = 0.f;
    isPlaying = true; // Or false depending on desired behavior
    if (!frames.empty()) {
        sprite.setTextureRect(frames[0]);
    }
}