#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <list>
#include <memory>
#include "Entity.h"
#include "Player.h"
#include "Boss.h"
#include "Asteroid.h"

class ResourceManager;
// class Animation;
// class Asteroid;
// class Bullet;
// class PowerUp;
// class Effect;
class HazardMeteor; // Forward declare

class Game {
public:
    enum class State { MainMenu, Instructions, Story, Playing, LevelTransition, Paused, GameOver }; // Added Story
    enum class PlayMode { Campaign, Survival };

    Game();
    ~Game();

    void run();

private:
    sf::RenderWindow window;
    sf::Clock clock;

    State currentState;
    PlayMode currentMode;

    ResourceManager& resourceManager;
    std::list<std::unique_ptr<Entity>> entities;
    Player* player;
    Boss* currentBoss; // Pointer to the current boss

    // --- Animations (Load once) ---
    // Player (Placeholder - Requires updated spritesheet)
    // Animation animPlayerIdle;
    // Animation animPlayerThrust;
    // Asteroids
    Animation animRockLarge;
    Animation animRockMedium;
    Animation animRockSmall;
    // Bullets
    Animation animBulletBlue;
    Animation animBulletRed;
    Animation animBulletLaser;
    // Powerups/Downs (mostly single frame, loaded dynamically in settings for now)
    // Animation animShieldPU;
    // Animation animWeaponPU;
    // Animation animSpeedPU;
    Animation animHazardMeteor; // Slow meteor anim
    // Explosions
    Animation animExplosionAsteroid; // Type C
    Animation animExplosionPlayer;   // Type B
    Animation animExplosionSmall;    // Type A
    Animation animExplosionBoss;
    // Boss
    Animation animBoss1;
    // Animation animBoss2; // If you add Boss 2

    // --- Game variables ---
    int currentLevel;
    float asteroidSpawnTimer;
    float powerUpSpawnTimer;
    float hazardMeteorSpawnTimer; // Timer for slow meteors
    float playerRespawnTimer;
    int bossDefeatScoreBonus;
    float storyDisplayTimer; // Timer for showing story text

    // --- UI Elements ---
    sf::Font uiFont;
    sf::Text scoreText;
    sf::Text livesText;
    sf::Text messageText;
    sf::Text levelText;
    sf::Sprite gameOverSprite; // Sprite for Game Over image

    // --- Sounds ---
    sf::Sound shootSound;
    sf::Sound explosionSoundAsteroid;
    sf::Sound explosionSoundPlayer;
    sf::Sound powerupSound;
    sf::Sound powerdownSound; // Sound for slow meteor hit
    sf::Sound bossHitSound; // TODO: Add sound file
    sf::Sound bossExplodeSound; // TODO: Add sound file
    sf::Music backgroundMusic;
    sf::Music bossMusic;

    // --- Methods ---
    void initialize();
    void loadResources(); // Loads textures/sounds AND creates Animation objects
    void setupUI();
    void setState(State newState);

    void handleInput();
    void update(float dt);
    void render();

    // State updates
    void updateMainMenu(float dt);
    void updatePlaying(float dt);
    void updateLevelTransition(float dt);
    void updateGameOver(float dt);
    void updateStory(float dt);

    // Render states
    void renderMainMenu();
    void renderPlaying();
    void renderLevelTransition();
    void renderGameOver();
    void renderInstructions();
    void renderStory();
    void renderPaused();

    // Game logic helpers
    void loadLevel(int levelNum);
    void startSurvival();
    void resetGame(bool fullReset = false); // Add flag for partial reset (keep score/level)
    void spawnPlayer(); // Helper to create/add player
    void spawnAsteroid(Asteroid::Size size, sf::Vector2f pos = {-100, -100});
    void spawnBullet();
    void spawnPowerUp();
    void spawnHazardMeteor();
    void spawnEffect(Animation& anim, sf::Vector2f pos);
    void spawnBoss(int level); // Spawn boss based on level
    void spawnBossBullet(Boss* boss, int firePointIndex); // Boss shooting logic
    void triggerBossExplosion(sf::Vector2f bossPos); // Handle boss death effect

    void checkCollisions();
    void cleanupEntities();

    bool checkLevelComplete();
    void nextLevel();

    void showInstructions();
    void showStory(int level); // Show story based on level

    static bool isCollide(const Entity *a, const Entity *b);
};

#endif // GAME_H