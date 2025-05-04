#include "ResourceManager.h"
#include <iostream> // For error messages

// Initialize static instance (Singleton pattern)
ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

sf::Texture& ResourceManager::getTexture(const std::string& filename) {
    // Check if texture is already loaded
    auto it = textures.find(filename);
    if (it != textures.end()) {
        return *it->second;
    }

    // Load texture if not found
    auto texture = std::make_unique<sf::Texture>();
    std::string fullPath = basePath + filename; // Assuming textures are in 'images/' relative to exe
    if (!texture->loadFromFile(fullPath)) {
        throw std::runtime_error("Failed to load texture: " + fullPath);
    }
    std::cout << "Loaded texture: " << fullPath << std::endl;
    // texture->setSmooth(true); // Optional smoothing
    textures[filename] = std::move(texture);
    return *textures[filename];
}

sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& filename) {
    auto it = soundBuffers.find(filename);
    if (it != soundBuffers.end()) {
        return *it->second;
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    std::string fullPath = soundPath + filename; // Assuming sounds are in 'sounds/'
    if (!buffer->loadFromFile(fullPath)) {
        throw std::runtime_error("Failed to load sound buffer: " + fullPath);
    }
     std::cout << "Loaded sound buffer: " << fullPath << std::endl;
    soundBuffers[filename] = std::move(buffer);
    return *soundBuffers[filename];
}

sf::Font& ResourceManager::getFont(const std::string& filename) {
    auto it = fonts.find(filename);
    if (it != fonts.end()) {
        return *it->second;
    }

    auto font = std::make_unique<sf::Font>();
    // std::string fullPath = fontPath + filename; // Assuming fonts are in 'fonts/'
    std::string fullPath = filename; // Or adjust path as needed
    if (!font->loadFromFile(fullPath)) {
         throw std::runtime_error("Failed to load font: " + fullPath);
    }
     std::cout << "Loaded font: " << fullPath << std::endl;
    fonts[filename] = std::move(font);
    return *fonts[filename];
}