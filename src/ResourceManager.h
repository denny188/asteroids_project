#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>

class ResourceManager {
public:
    ResourceManager() = default; // Default constructor

    // Prevent copying and assignment
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    static ResourceManager& getInstance(); // Singleton access

    sf::Texture& getTexture(const std::string& filename);
    sf::SoundBuffer& getSoundBuffer(const std::string& filename);
    sf::Font& getFont(const std::string& filename);

private:
    std::map<std::string, std::unique_ptr<sf::Texture>> textures;
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    std::map<std::string, std::unique_ptr<sf::Font>> fonts;

    // Base path for assets (adjust if needed)
    std::string basePath = "images/"; // Default for textures
    std::string soundPath = "sounds/";
    // Add font path if needed
};

#endif // RESOURCEMANAGER_H