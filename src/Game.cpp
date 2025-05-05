#include "Game.h"
#include "ResourceManager.h"
#include "Animation.h"
#include "Asteroid.h"
#include "Bullet.h"
#include "PowerUp.h"
#include "HazardMeteor.h"
#include "Effect.h"
#include "Boss.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream> // Required for file I/O
#include <limits>  // Required for numeric_limits (though not used directly now)

// --- Constants ---
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const float ASTEROID_SPAWN_RATE_BASE = 3.5f;
const float POWERUP_SPAWN_RATE_BASE = 12.0f;
const float HAZARD_METEOR_SPAWN_RATE = 15.0f;
const float PLAYER_RESPAWN_DELAY = 3.0f;
const float STORY_DISPLAY_DURATION = 4.0f;
const int BOSS_LEVEL_INTERVAL = 3;
const std::string HIGHSCORE_FILE = "highscore.dat";

// --- Constructor ---
Game::Game() :
    window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Asteroids Game"),
    currentState(State::MainMenu), // State được khởi tạo ở đây
    currentMode(PlayMode::Campaign),
    resourceManager(ResourceManager::getInstance()),
    player(nullptr),
    currentBoss(nullptr),
    selectedShipType(Player::ShipType::Standard),
    currentLevel(0),
    asteroidSpawnTimer(ASTEROID_SPAWN_RATE_BASE),
    powerUpSpawnTimer(POWERUP_SPAWN_RATE_BASE),
    hazardMeteorSpawnTimer(HAZARD_METEOR_SPAWN_RATE),
    playerRespawnTimer(0.f),
    bossDefeatScoreBonus(1000),
    storyDisplayTimer(0.f),
    highScore(0)
{
    std::cout << "Game Constructor: Initializing window..." << std::endl; // DEBUG
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    std::cout << "Game Constructor: Calling initialize()..." << std::endl; // DEBUG
    initialize();
    std::cout << "Game Constructor: initialize() finished." << std::endl; // DEBUG
}

// --- Destructor ---
Game::~Game() {
    backgroundMusic.stop();
    bossMusic.stop();
}

// --- Initialization ---
void Game::initialize() {
    std::cout << "initialize() called." << std::endl; // DEBUG
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::cout << " - Loading resources..." << std::endl; // DEBUG
    loadResources();
    std::cout << " - Loading high score..." << std::endl; // DEBUG
    loadHighScore();
    std::cout << " - Setting up UI..." << std::endl; // DEBUG
    setupUI();
    std::cout << " - Setting initial state to MainMenu..." << std::endl; // DEBUG
    // Gọi setState một cách tường minh thay vì dựa vào giá trị khởi tạo ban đầu
    // currentState = State::MainMenu; // Gán trực tiếp có thể bỏ qua logic trong setState
    setState(State::MainMenu); // Gọi hàm setState để thực thi logic thiết lập
    std::cout << "initialize() finished." << std::endl; // DEBUG
}

// --- Resource Loading ---
void Game::loadResources() {
    std::cout << "Loading resources and animations..." << std::endl;
    try {
        // Textures
        resourceManager.getTexture("spaceship.png");
        resourceManager.getTexture("background.jpg");
        resourceManager.getTexture("rock.png");
        resourceManager.getTexture("rock_medium.png");
        resourceManager.getTexture("rock_small.png");
        resourceManager.getTexture("fire_blue.png");
        resourceManager.getTexture("fire_red.png");
        resourceManager.getTexture("fire_laser.png");
        resourceManager.getTexture("slow_powerdown.png");
        resourceManager.getTexture("shield_powerup.png");
        resourceManager.getTexture("weapon_powerup.png");
        resourceManager.getTexture("speed_powerup.png");
        resourceManager.getTexture("boss1.png");
        resourceManager.getTexture("gameover.png");
        resourceManager.getTexture("explosions/type_A.png");
        resourceManager.getTexture("explosions/type_B.png");
        resourceManager.getTexture("explosions/type_C.png");
        resourceManager.getTexture("explosions/boss_explosion.png");

        // Animations
        animRockLarge = Animation(resourceManager.getTexture("rock.png"), 0, 0, 64, 64, 16, 0.2f);
        animRockMedium = Animation(resourceManager.getTexture("rock_medium.png"), 0, 0, 96, 96, 12, 0.25f);
        animRockSmall = Animation(resourceManager.getTexture("rock_small.png"), 0, 0, 64, 64, 16, 0.3f);
        animBulletBlue = Animation(resourceManager.getTexture("fire_blue.png"), 0, 0, 32, 64, 16, 0.8f, false);
        animBulletRed = Animation(resourceManager.getTexture("fire_red.png"), 0, 0, 32, 64, 16, 0.9f, false);
        animBulletLaser = Animation(resourceManager.getTexture("fire_laser.png"), 0, 0, 64, 64, 18, 1.2f, false);
        animHazardMeteor = Animation(resourceManager.getTexture("slow_powerdown.png"), 0, 0, 64, 64, 24, 0.3f, true);
        animExplosionSmall = Animation(resourceManager.getTexture("explosions/type_A.png"), 0, 0, 51, 50, 20, 0.6f, false);
        animExplosionPlayer = Animation(resourceManager.getTexture("explosions/type_B.png"), 0, 0, 192, 192, 64, 0.7f, false);
        animExplosionAsteroid = Animation(resourceManager.getTexture("explosions/type_C.png"), 0, 0, 256, 256, 48, 0.6f, false);
        animExplosionBoss = Animation(resourceManager.getTexture("explosions/boss_explosion.png"), 0, 0, 64, 64, 8, 0.5f, false);
        animBoss1 = Animation(resourceManager.getTexture("boss1.png"), 0, 0, 230, 336, 1, 0, false);

        // Fonts (Adjust path as needed - place font near executable or provide full path)
        if (!uiFont.loadFromFile("arial.ttf")) { // Example: Assuming arial.ttf is in the same folder
             // Try Windows path as fallback, but ideally the font is local
             if (!uiFont.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
                 throw std::runtime_error("Failed to load font: arial.ttf (checked local and C:/Windows/Fonts)");
             }
        }

        // Sounds
        shootSound.setBuffer(resourceManager.getSoundBuffer("shoot.wav"));
        explosionSoundAsteroid.setBuffer(resourceManager.getSoundBuffer("explosion_asteroid.wav"));
        explosionSoundPlayer.setBuffer(resourceManager.getSoundBuffer("explosion_player.wav"));
        powerupSound.setBuffer(resourceManager.getSoundBuffer("powerup_collect.wav"));
        powerdownSound.setBuffer(resourceManager.getSoundBuffer("powerdown.ogg"));
        // bossHitSound.setBuffer(resourceManager.getSoundBuffer("boss_hit.wav"));
        // bossExplodeSound.setBuffer(resourceManager.getSoundBuffer("boss_explode.wav"));

        // Music
        if (!backgroundMusic.openFromFile("sounds/background_music.ogg")) throw std::runtime_error("Failed to load background music");
        backgroundMusic.setLoop(true); backgroundMusic.setVolume(30);
        if (!bossMusic.openFromFile("sounds/boss_theme.ogg")) throw std::runtime_error("Failed to load boss music");
        bossMusic.setLoop(true); bossMusic.setVolume(45);

    } catch (const std::exception& e) {
        std::cerr << "Error loading resources: " << e.what() << std::endl;
        // Consider closing the window or handling the error more gracefully
         window.close();
         exit(EXIT_FAILURE); // Exit if critical resources fail
    }
    std::cout << "Resources loaded successfully." << std::endl;
}

// --- UI Setup ---
void Game::setupUI() {
    scoreText.setFont(uiFont); scoreText.setCharacterSize(24); scoreText.setFillColor(sf::Color::White); scoreText.setPosition(10, 10);
    livesText.setFont(uiFont); livesText.setCharacterSize(24); livesText.setFillColor(sf::Color::White); livesText.setPosition(10, 40);
    levelText.setFont(uiFont); levelText.setCharacterSize(24); levelText.setFillColor(sf::Color::White); levelText.setPosition(window.getSize().x - 150.f, 10);
    messageText.setFont(uiFont); messageText.setCharacterSize(40); messageText.setFillColor(sf::Color::White);

    highScoreText.setFont(uiFont); highScoreText.setCharacterSize(20); highScoreText.setFillColor(sf::Color::Yellow);
    shipSelectionText.setFont(uiFont); shipSelectionText.setCharacterSize(20); shipSelectionText.setFillColor(sf::Color::Cyan);

    // Game Over Sprite
    try {
        gameOverSprite.setTexture(resourceManager.getTexture("gameover.png"));
        gameOverSprite.setOrigin(gameOverSprite.getLocalBounds().width / 2.f, gameOverSprite.getLocalBounds().height / 2.f);
        gameOverSprite.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 50);
    } catch (const std::runtime_error& e) {
        std::cerr << "Warning: Failed to load gameover.png texture. Game over screen will use text only." << std::endl;
    }
}

// --- State Management ---
void Game::setState(State newState) {
    State oldState = currentState;
     std::cout << "setState() called: old=" << static_cast<int>(oldState) << ", new=" << static_cast<int>(newState) << std::endl; // DEBUG

    currentState = newState;
    messageText.setString("");

    // State Exit Actions
    if (oldState == State::Playing || oldState == State::Paused) {
         std::cout << " - Pausing music due to exiting Playing/Paused." << std::endl; // DEBUG
        backgroundMusic.pause();
        bossMusic.pause();
    }

    // State Entry Actions
    std::cout << " - Entering setup for state " << static_cast<int>(currentState) << std::endl; // DEBUG
    switch (currentState) {
        case State::MainMenu:
             std::cout << "   - Setting up MainMenu..." << std::endl; // DEBUG
            resetGame(true); // Reset game state, spawns player
             std::cout << "   - Game reset complete (player spawned)." << std::endl; // DEBUG
            messageText.setString("ASTEROIDS DELUXE\n\n[P] Play Campaign\n[S] Play Survival\n[I] Instructions\n[N] Next Ship\n[Esc] Exit");
            messageText.setCharacterSize(40);
            // Origin/Positioning (Quan trọng: đảm bảo font đã load và string đã set)
            if (uiFont.getInfo().family.empty()) {
                 std::cerr << "   - WARNING: uiFont seems invalid in setState(MainMenu)!" << std::endl;
            }
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);

            updateShipSelectionText();
            shipSelectionText.setOrigin(shipSelectionText.getLocalBounds().left + shipSelectionText.getLocalBounds().width / 2.f, shipSelectionText.getLocalBounds().top + shipSelectionText.getLocalBounds().height / 2.f);
            shipSelectionText.setPosition(window.getSize().x / 2.f, messageText.getPosition().y + messageText.getGlobalBounds().height / 2.f + 40.f);

            highScoreText.setString("High Score: " + std::to_string(highScore));
            highScoreText.setOrigin(highScoreText.getLocalBounds().left + highScoreText.getLocalBounds().width, 0);
            highScoreText.setPosition(window.getSize().x - 10.f, 10.f);

             std::cout << "   - UI text set. Playing music..." << std::endl; // DEBUG
            if (backgroundMusic.getStatus() != sf::Music::Playing) {
                 backgroundMusic.play();
                 std::cout << "   - Background music started." << std::endl; // DEBUG
            } else {
                 std::cout << "   - Background music already playing." << std::endl; // DEBUG
            }
             std::cout << "   - MainMenu setup complete." << std::endl; // DEBUG
            break;

        case State::Instructions:
            showInstructions();
            break;

        case State::Story:
            // Text is set by showStory before this state is entered
            storyDisplayTimer = STORY_DISPLAY_DURATION;
            messageText.setCharacterSize(30);
            // Re-center after text is set
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(window.getSize().x / 2.f, window.getSize().y * 0.8f);
            break;

        case State::Playing:
             if (oldState == State::Paused) { // Resuming game
                 if (currentBoss) bossMusic.play(); else backgroundMusic.play();
             } else if (oldState == State::MainMenu || oldState == State::GameOver) { // Starting new game
                 // resetGame(true) was called in MainMenu or is handled by Retry/R key logic
                 // Need to initiate the chosen mode
                 if (currentMode == PlayMode::Campaign) {
                     currentLevel = 1; // Set level before showing story
                     showStory(currentLevel); // Will set state to Story or Playing
                 } else { // Survival
                     currentLevel = 1;
                     startSurvival();
                     // If startSurvival doesn't set state, set it here
                     if (currentState != State::Playing) setState(State::Playing);
                 }
                 // Music is handled by specific start/load functions or story transition
             } else if (oldState == State::Story || oldState == State::LevelTransition) { // Coming from story/transition
                 // Level was loaded by updateStory or updateLevelTransition calling loadLevel
                 // Ensure music is correct
                 if (currentBoss) { if(bossMusic.getStatus() != sf::Music::Playing) bossMusic.play(); }
                 else { if(backgroundMusic.getStatus() != sf::Music::Playing) backgroundMusic.play(); }
             } else if (player && !player->life && player->lives > 0) { // Respawning state triggered by updatePlaying
                 playerRespawnTimer = PLAYER_RESPAWN_DELAY;
                 if (currentBoss) { if(bossMusic.getStatus() != sf::Music::Playing) bossMusic.play(); }
                 else { if(backgroundMusic.getStatus() != sf::Music::Playing) backgroundMusic.play(); }
             }
            break;

        case State::LevelTransition:
            messageText.setString("Level " + std::to_string(currentLevel -1) + " Complete!"); // Show level just completed
            messageText.setCharacterSize(40);
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
            clock.restart(); // Start timer for transition delay
            break;

        case State::GameOver:
            if (player && player->score > highScore) {
                highScore = player->score;
                saveHighScore();
            }
            messageText.setCharacterSize(30);
            messageText.setString("\n\nFinal Score: " + std::to_string(player ? player->score : 0) +
                                  "\nHigh Score: " + std::to_string(highScore) +
                                  "\n\n[R] Retry\n[M] Main Menu");
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(gameOverSprite.getPosition().x, gameOverSprite.getPosition().y + gameOverSprite.getGlobalBounds().height / 2.f + 50.f);
            backgroundMusic.stop(); // Ensure music stops
            bossMusic.stop();
            break;

        case State::Paused:
            messageText.setString("PAUSED\n\n[Esc] Resume\n[M] Main Menu");
            messageText.setCharacterSize(40);
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
            // Music pause handled by exit actions of Playing state
            break;
    }
    std::cout << "setState() finished for state " << static_cast<int>(currentState) << std::endl; // DEBUG
}

// --- Main Loop ---
void Game::run() {
    std::cout << "Starting main game loop..." << std::endl; // DEBUG
    while (window.isOpen()) {
        // 1. Calculate Delta Time
        float dt = clock.restart().asSeconds();
        // Clamp delta time to prevent issues after pauses or lag spikes
        if (dt > 0.1f) dt = 0.1f;

        // 2. Handle Input (MUST BE CALLED EVERY FRAME)
        // std::cout << "Calling handleInput()..." << std::endl; // DEBUG (Optional, can be noisy)
        handleInput();

        // 3. Update Game State (MUST BE CALLED EVERY FRAME)
        // std::cout << "Calling update(" << dt << ")..." << std::endl; // DEBUG (Optional, can be noisy)
        update(dt);

        // 4. Render Graphics (MUST BE CALLED EVERY FRAME)
        // std::cout << "Calling render()..." << std::endl; // DEBUG (Optional, can be noisy)
        render(); // render() calls window.display() internally
    }
     std::cout << "Exited main game loop." << std::endl; // DEBUG
}

// --- Input Handling ---
void Game::handleInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            return; // Exit polling loop if window closed
        }

        // Global Keys
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                if (currentState == State::Playing) setState(State::Paused);
                else if (currentState == State::Paused) setState(State::Playing);
                else if (currentState == State::Instructions) setState(State::MainMenu);
                else if (currentState == State::Story) { /* Allow skipping story? setState(State::Playing); loadLevel(currentLevel); */ }
                else if (currentState == State::MainMenu) window.close();
            }
            if (event.key.code == sf::Keyboard::M) {
                if (currentState == State::Paused || currentState == State::GameOver) {
                    setState(State::MainMenu);
                }
            }
        }

        // State-Specific Inputs
        switch (currentState) {
            case State::MainMenu:
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::P) { currentMode = PlayMode::Campaign; setState(State::Playing); }
                    else if (event.key.code == sf::Keyboard::S) { currentMode = PlayMode::Survival; setState(State::Playing); }
                    else if (event.key.code == sf::Keyboard::I) { setState(State::Instructions); }
                    else if (event.key.code == sf::Keyboard::N) {
                        cycleShipSelection();
                        updateShipSelectionText();
                        // Re-center ship text
                        shipSelectionText.setOrigin(shipSelectionText.getLocalBounds().left + shipSelectionText.getLocalBounds().width / 2.f, shipSelectionText.getLocalBounds().top + shipSelectionText.getLocalBounds().height / 2.f);
                        shipSelectionText.setPosition(window.getSize().x / 2.f, messageText.getPosition().y + messageText.getGlobalBounds().height / 2.f + 40.f);
                    }
                }
                break;

            case State::Playing:
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space && player && player->life) {
                        player->shoot(); // Signal intent
                        if (player->shootTimer <= 0) { // Check cooldown *before* spawning
                            spawnBullet();
                        }
                    }
                }
                // Player movement input is handled directly in Player::handleInput using isKeyPressed
                break;

            case State::GameOver:
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                         // Reset is handled by setState(State::Playing) when coming from GameOver
                         setState(State::Playing);
                    }
                }
                break;

            case State::Instructions: // Esc handled globally
            case State::Story:        // Waits for timer or Esc (potential skip)
            case State::LevelTransition: // Waits for timer
            case State::Paused:       // Esc/M handled globally
                 break;
        }
    }
}

// --- Update Dispatcher ---
void Game::update(float dt) {
    switch (currentState) {
        case State::Playing: updatePlaying(dt); break;
        case State::LevelTransition: updateLevelTransition(dt); break;
        case State::Story: updateStory(dt); break;
        // MainMenu, GameOver, Instructions, Paused don't have continuous updates
        case State::MainMenu:        /* updateMainMenu(dt); */ break;
        case State::GameOver:        /* updateGameOver(dt); */ break;
        case State::Instructions:    break;
        case State::Paused:          break;
    }
}

// --- State Update Implementations ---

void Game::updateStory(float dt) {
    storyDisplayTimer -= dt;
    if (storyDisplayTimer <= 0) {
        // Story finished, load level and transition to Playing
        loadLevel(currentLevel); // Load the actual level content
        setState(State::Playing); // Transition to playing state
    }
}

void Game::updateLevelTransition(float dt) {
    if (clock.getElapsedTime().asSeconds() > 2.0f) {
        // Transition finished, show story for the *next* level
        showStory(currentLevel); // showStory handles the next state (Story or Playing)
    }
}

void Game::updatePlaying(float dt) {
    // 1. Handle Player Respawn Timer
    if (playerRespawnTimer > 0) {
        playerRespawnTimer -= dt;
        if (playerRespawnTimer <= 0) {
            if (player && player->lives > 0) { // Player was dead but has lives left
                player->reset(); // Reset stats (pos, velocity, effects etc.)
                player->pos = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
                player->life = true; // Revive
                player->shieldActive = true; // Respawn shield
                player->shieldTimer = 2.0f;
            } else if (!player) {
                 // This case shouldn't ideally happen if logic is correct,
                 // means player unique_ptr got deleted before respawn timer finished.
                 // Maybe game over was triggered prematurely?
                 // For safety, just transition to game over if player is gone.
                 std::cerr << "Error: Respawn timer ended but player pointer is null." << std::endl;
                 setState(State::GameOver);
                 return;
            }
            // If player lives <= 0, the game over state would have been set earlier
        } else {
            return; // Still waiting for respawn, skip rest of update
        }
    }

    // Check if player is null and not respawning -> Game Over
    if (!player && playerRespawnTimer <= 0) {
        if (currentState != State::GameOver) { // Prevent multiple calls
             std::cout << "Player is null and not respawning. Triggering Game Over." << std::endl;
             setState(State::GameOver);
        }
        return; // Stop updatePlaying if game over
    }

    // 2. Handle Spawning
    // Asteroids (only if no boss)
    if (!currentBoss || !currentBoss->life) {
        asteroidSpawnTimer -= dt;
        if (asteroidSpawnTimer <= 0) {
            int sizeRoll = rand() % 3;
            Asteroid::Size spawnSize = (sizeRoll == 0) ? Asteroid::Size::Large : ((sizeRoll == 1) ? Asteroid::Size::Medium : Asteroid::Size::Small);
            spawnAsteroid(spawnSize);
            asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE / (1.0f + currentLevel * 0.05f); // Increase rate slightly with level
            if (asteroidSpawnTimer < 0.5f) asteroidSpawnTimer = 0.5f; // Cap spawn rate
        }
    }

    // Hazard Meteors
    hazardMeteorSpawnTimer -= dt;
    if (hazardMeteorSpawnTimer <= 0) {
        spawnHazardMeteor();
        hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE * (0.8f + static_cast<float>(rand() % 40) / 100.f); // Randomize slightly
    }

    // Power-ups
    powerUpSpawnTimer -= dt;
    if (powerUpSpawnTimer <= 0) {
        spawnPowerUp();
        powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE * (0.9f + static_cast<float>(rand() % 20) / 100.f); // Randomize slightly
    }

    // 3. Update Entities
    // Use explicit iterator loop to handle potential removals during update (though cleanup is separate)
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        Entity* e = it->get();
        if (e->life) {
            e->update(dt, window.getSize());

            // Trigger Boss shooting based on its internal timers
            if (e->type == Entity::Type::Boss) {
                Boss* boss = static_cast<Boss*>(e);
                // Boss timers are decremented in Boss::updateShooting
                // Game::spawnBossBullet checks if timer <= 0
                if (boss->shootTimer1 <= 0) { spawnBossBullet(boss, 0); /* Boss resets its own timer */ }
                if (boss->currentPhase > 0 && boss->shootTimer2 <= 0) { spawnBossBullet(boss, 1); }
                if (boss->currentPhase > 1 && boss->shootTimer3 <= 0) { spawnBossBullet(boss, 2); }
            }
        }
    }

    // 4. Check Collisions
    checkCollisions();

    // 5. Cleanup Entities marked as not alive
    cleanupEntities();

    // 6. Update UI Text
    if (player) { // Check if player still exists after cleanup
        scoreText.setString("Score: " + std::to_string(player->score));
        livesText.setString("Lives: " + std::to_string(player->lives));
    } else {
        // If player is gone and we are not yet in GameOver state (and not respawning), something's wrong.
        // This case is now handled earlier by checking !player && playerRespawnTimer <= 0.
        // If we reach here and player is null, it means cleanup just removed them.
        // UI will update correctly on the next frame when state is GameOver.
        scoreText.setString("Score: ---");
        livesText.setString("Lives: 0");
    }
    levelText.setString(((currentMode == PlayMode::Campaign) ? "Level: " : "Wave: ") + std::to_string(currentLevel));

    // 7. Check Level Completion (Campaign Mode Only)
    if (currentMode == PlayMode::Campaign) {
        bool levelDone = checkLevelComplete();
        if (levelDone) {
            // Can proceed if player is alive or if they are dead but have finished respawning (timer <= 0)
            bool canProceed = (player && player->life) || playerRespawnTimer <= 0;
            if (!currentBoss && canProceed) { // Ensure no boss AND player ready
                nextLevel(); // Increment level counter
                setState(State::LevelTransition);
                return; // Exit updatePlaying early as state has changed
            }
        }
    }
     // --- Make sure no other logic runs if state changed during update ---
     if (currentState != State::Playing) return;
}

// --- Rendering Dispatcher ---
void Game::render() {
    // std::cout << "render() called. Current State: " << static_cast<int>(currentState) << std::endl; // DEBUG
    window.clear(sf::Color::Black);

    // Draw Background
    try {
        sf::Sprite backgroundSprite(resourceManager.getTexture("background.jpg"));
        backgroundSprite.setScale(
            static_cast<float>(window.getSize().x) / backgroundSprite.getLocalBounds().width,
            static_cast<float>(window.getSize().y) / backgroundSprite.getLocalBounds().height);
        window.draw(backgroundSprite);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error rendering background: " << e.what() << std::endl;
    }

    // Draw Entities (Effects first for layering)
    for (const auto& entity : entities) {
        if (entity->type == Entity::Type::Effect) entity->draw(window);
    }
    for (const auto& entity : entities) {
        // Draw all non-effects, excluding player (drawn last)
        if (entity->type != Entity::Type::Effect && entity.get() != player) entity->draw(window);
    }
    // Draw player last if alive (handles overlays internally)
    if (player) {
        // std::cout << "Attempting to draw player. Life: " << player->life << std::endl; // DEBUG
        if (player->life) {
            player->draw(window);
        }
    } else {
        std::cout << "Player pointer is null, cannot draw." << std::endl; // DEBUG
    }


    // Draw UI / Messages based on state
    switch (currentState) {
        case State::MainMenu:       renderMainMenu(); break;
        case State::Instructions:   renderInstructions(); break;
        case State::Story:          renderStory(); break;
        case State::Playing:        renderPlaying(); break;
        case State::LevelTransition:renderLevelTransition(); break;
        case State::GameOver:       renderGameOver(); break;
        case State::Paused:         renderPaused(); break;
    }

    window.display();
}

// --- State Render Implementations ---

void Game::renderMainMenu() {
    // std::cout << "renderMainMenu() called." << std::endl; // DEBUG
    // Kiểm tra xem font có hợp lệ không trước khi vẽ
    if (uiFont.getInfo().family.empty()) {
         std::cerr << "WARNING: Attempting to render MainMenu with invalid font!" << std::endl;
         // Vẽ hình chữ nhật thay thế để biết hàm có chạy không
         sf::RectangleShape rect(sf::Vector2f(200, 100));
         rect.setFillColor(sf::Color::Red);
         rect.setPosition(100, 100);
         window.draw(rect);
         return; // Không vẽ text nếu font lỗi
    }
    // std::cout << " - Drawing messageText..." << std::endl; // DEBUG
    window.draw(messageText);
    // std::cout << " - Drawing shipSelectionText..." << std::endl; // DEBUG
    window.draw(shipSelectionText);
    // std::cout << " - Drawing highScoreText..." << std::endl; // DEBUG
    window.draw(highScoreText);
}

void Game::renderInstructions() {
    window.draw(messageText); // Assumes text set in showInstructions
}

void Game::renderStory() {
    window.draw(messageText); // Assumes text set before entering state
}

void Game::renderPlaying() {
    window.draw(scoreText);
    window.draw(livesText);
    window.draw(levelText);

    // Draw boss health bar if boss exists and is alive
    if (currentBoss && currentBoss->life) {
        float barWidth = 300.f;
        float barHeight = 15.f;
        float healthPercent = static_cast<float>(currentBoss->health) / currentBoss->maxHealth;
        if (healthPercent < 0) healthPercent = 0; // Clamp health display

        sf::RectangleShape backgroundBar(sf::Vector2f(barWidth, barHeight));
        backgroundBar.setFillColor(sf::Color(100, 100, 100, 200));
        backgroundBar.setPosition(window.getSize().x / 2.f - barWidth / 2.f, 20.f);

        sf::RectangleShape healthBar(sf::Vector2f(barWidth * healthPercent, barHeight));
        healthBar.setFillColor(sf::Color(200, 0, 0, 200));
        healthBar.setPosition(window.getSize().x / 2.f - barWidth / 2.f, 20.f);

        window.draw(backgroundBar);
        window.draw(healthBar);
    }
}

void Game::renderLevelTransition() {
    renderPlaying(); // Show game stats behind message
    window.draw(messageText);
}

void Game::renderGameOver() {
    if (gameOverSprite.getTexture()) { // Draw sprite only if texture loaded successfully
        window.draw(gameOverSprite);
    }
    window.draw(messageText); // Draw score/options text
}

void Game::renderPaused() {
    renderPlaying(); // Draw the paused game state underneath

    // Draw semi-transparent overlay
    sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
    overlay.setFillColor(sf::Color(0, 0, 0, 150)); // Black with alpha
    window.draw(overlay);

    // Draw Pause message text on top
    window.draw(messageText);
}


// --- Game Logic Helpers ---

void Game::loadLevel(int levelNum) {
    std::cout << "--- Loading Level: " << levelNum << " ---" << std::endl;
    resetGame(false); // Partial reset (keeps score, lives, selected ship)

    // Player is guaranteed to exist after resetGame(false) calls spawnPlayer

    int numAsteroids = 4 + levelNum;
    int numHazards = levelNum / 2; // Example scaling

    for (int i = 0; i < numAsteroids; ++i) {
        spawnAsteroid(Asteroid::Size::Large);
    }
    for (int i = 0; i < numHazards; ++i) {
        spawnHazardMeteor();
    }

    // Spawn Boss?
    if (currentMode == PlayMode::Campaign && levelNum > 0 && levelNum % BOSS_LEVEL_INTERVAL == 0) {
        spawnBoss(levelNum);
        backgroundMusic.stop();
        bossMusic.play();
    } else {
        // Ensure boss music is stopped and background music plays if no boss
        if (bossMusic.getStatus() == sf::Music::Playing) bossMusic.stop();
        if (backgroundMusic.getStatus() != sf::Music::Playing) backgroundMusic.play();
    }

    // Reset spawn timers for the new level
    asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE;
    powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE;
    hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE;
    playerRespawnTimer = 0.f; // Ensure player starts active

    std::cout << "--- Level " << levelNum << " loading complete. Entity count: " << entities.size() << " ---" << std::endl;
}

void Game::startSurvival() {
    std::cout << "Starting Survival Mode" << std::endl;
    resetGame(true); // Full reset for survival mode
    currentLevel = 1; // Survival starts at wave 1

    // Player should exist after resetGame(true)

    for (int i = 0; i < 3; ++i) { // Start with a few asteroids
        spawnAsteroid(Asteroid::Size::Large);
    }

    asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE;
    powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE;
    hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE;
    playerRespawnTimer = 0.f;

    bossMusic.stop(); // Ensure no boss music
    backgroundMusic.play();
    // Set state to Playing *after* setup is complete
    setState(State::Playing);
}

void Game::resetGame(bool fullReset) {
    // Store player stats if not a full reset and player exists
    int previousScore = 0;
    int previousLives = 3; // Default starting lives
    Player::ShipType shipToUse = selectedShipType; // Default to selection

    if (!fullReset && player) {
        previousScore = player->score;
        previousLives = player->lives;
        shipToUse = player->currentShipType; // Keep the ship they were using
    }

    // Clear all entities
    entities.clear();
    player = nullptr;
    currentBoss = nullptr;

    // Always respawn player object after clearing
    spawnPlayer(); // Creates the player object and sets the raw pointer

    if (fullReset) {
        currentLevel = 1; // Reset level for full reset
        if (player) {
            player->score = 0;
            player->lives = 3;
            player->setShipType(selectedShipType); // Use the globally selected type
        }
        selectedShipType = Player::ShipType::Standard; // Reset global selection
        updateShipSelectionText(); // Update menu display
    } else {
        // Keep currentLevel
        if (player) {
            player->score = previousScore;
            player->lives = previousLives;
            player->setShipType(shipToUse); // Restore the correct ship type
            player->reset(); // Reset position, velocity, effects etc.
            player->pos = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
            player->life = true; // Ensure player is alive
        }
    }
}

void Game::spawnPlayer() {
    // if (player) return; // Should be null after entities.clear() in resetGame

    auto newPlayer = std::make_unique<Player>();
    player = newPlayer.get(); // Get raw pointer BEFORE moving ownership

    Animation dummyAnim; // Player::settings loads its own textures/anims
    player->settings(dummyAnim, sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
    // Score/lives/ship type are handled by resetGame logic calling this

    entities.push_back(std::move(newPlayer)); // Add to entity list
    std::cout << "Player spawned/re-added." << std::endl;
}

void Game::spawnAsteroid(Asteroid::Size size, sf::Vector2f pos) {
    auto asteroid = std::make_unique<Asteroid>(size);
    Animation* animPtr = nullptr;
    float radius;

    switch(size) {
        case Asteroid::Size::Large:  animPtr = &animRockLarge; radius = 25.f; break;
        case Asteroid::Size::Medium: animPtr = &animRockMedium; radius = 15.f; break;
        case Asteroid::Size::Small:  animPtr = &animRockSmall; radius = 8.f; break;
        default: std::cerr << "Error: Invalid asteroid size requested!" << std::endl; return;
    }

    // Calculate random edge position if not provided
    if (pos.x == -100 && pos.y == -100) { // Use the default value as a flag
        int edge = rand() % 4;
        float spawnX = 0, spawnY = 0;
        switch(edge) {
            case 0: // Top
                spawnX = static_cast<float>(rand() % WINDOW_WIDTH);
                spawnY = -radius;
                break;
            case 1: // Right
                spawnX = static_cast<float>(WINDOW_WIDTH + radius);
                spawnY = static_cast<float>(rand() % WINDOW_HEIGHT);
                break;
            case 2: // Bottom
                spawnX = static_cast<float>(rand() % WINDOW_WIDTH);
                spawnY = static_cast<float>(WINDOW_HEIGHT + radius);
                break;
            case 3: // Left
                spawnX = -radius;
                spawnY = static_cast<float>(rand() % WINDOW_HEIGHT);
                break;
        }
        pos = sf::Vector2f(spawnX, spawnY);
    }

    asteroid->settings(*animPtr, pos, static_cast<float>(rand() % 360), radius);
    if (asteroid->type != Entity::Type::Asteroid) { // Sanity check after settings
        std::cerr << "Warning: Spawned asteroid does not have Asteroid type!" << std::endl;
    }
    entities.push_back(std::move(asteroid));
}

void Game::spawnHazardMeteor() {
     auto meteor = std::make_unique<HazardMeteor>();
     float radius = 20.f;
     sf::Vector2f pos;

     int edge = rand() % 4;
     float spawnX = 0, spawnY = 0;
     switch(edge) {
         case 0: spawnX = static_cast<float>(rand() % WINDOW_WIDTH); spawnY = -radius; break;
         case 1: spawnX = static_cast<float>(WINDOW_WIDTH + radius); spawnY = static_cast<float>(rand() % WINDOW_HEIGHT); break;
         case 2: spawnX = static_cast<float>(rand() % WINDOW_WIDTH); spawnY = static_cast<float>(WINDOW_HEIGHT + radius); break;
         case 3: spawnX = -radius; spawnY = static_cast<float>(rand() % WINDOW_HEIGHT); break;
     }
     pos = sf::Vector2f(spawnX, spawnY);

     meteor->settings(animHazardMeteor, pos, static_cast<float>(rand() % 360), radius);
      if (meteor->type != Entity::Type::HazardMeteor) { // Sanity check
        std::cerr << "Warning: Spawned hazard meteor does not have HazardMeteor type!" << std::endl;
      }
     entities.push_back(std::move(meteor));
}

void Game::spawnBullet() {
    // Cooldown check is done in handleInput before calling this
    if (!player || !player->life) {
        std::cerr << "SpawnBullet called but player is null or dead." << std::endl;
        return;
    }

    // Reset cooldown and play sound *now* that we know we are spawning
    player->shootTimer = player->shootCooldown;
    shootSound.play();

    Bullet::BulletType typeToSpawn = player->currentWeaponType;
    Animation* animPtr = nullptr;
    int bulletsToSpawn = 1;
    float spreadAngle = 15.f; // Degrees for spread shot

    switch(typeToSpawn) {
        case Bullet::BulletType::Standard: animPtr = &animBulletBlue; bulletsToSpawn = 1; break;
        case Bullet::BulletType::Laser:    animPtr = &animBulletLaser; bulletsToSpawn = 1; break;
        case Bullet::BulletType::Spread:   animPtr = &animBulletBlue; bulletsToSpawn = 3; break;
        case Bullet::BulletType::Red:      animPtr = &animBulletRed; bulletsToSpawn = 1; break; // Ensure Red is intended for player
        default: std::cerr << "Error: Unknown bullet type requested!" << std::endl; return;
    }

    if (!animPtr) { // Should not happen if switch is exhaustive
         std::cerr << "Error: Could not find animation pointer for bullet type " << static_cast<int>(typeToSpawn) << std::endl;
         player->shootTimer = 0; // Allow immediate retry if anim failed
         return;
    }

    float baseAngle = player->angle;
    float offsetDist = player->R + 5.f; // Spawn slightly in front
    float angleRadBase = (baseAngle - 90) * 3.14159f / 180.f;
    sf::Vector2f offsetVecBase = sf::Vector2f(std::cos(angleRadBase) * offsetDist, std::sin(angleRadBase) * offsetDist);
    sf::Vector2f spawnPosBase = player->pos + offsetVecBase;

    for (int i = 0; i < bulletsToSpawn; ++i) {
        auto bullet = std::make_unique<Bullet>(typeToSpawn);
        float shotAngle = baseAngle;
        if (bulletsToSpawn > 1) {
            // Calculate angle for spread: -(n-1)/2 * spread, ..., 0, ..., +(n-1)/2 * spread
            shotAngle += (static_cast<float>(i) - (static_cast<float>(bulletsToSpawn - 1) / 2.0f)) * spreadAngle;
        }

        // Note: For simplicity, spread shots originate from the same point.
        // Could adjust spawnPos slightly based on shotAngle if desired.
        bullet->settings(*animPtr, spawnPosBase, shotAngle);
        if (bullet->type != Entity::Type::Bullet) { // Sanity check
             std::cerr << "Warning: Spawned bullet does not have Bullet type!" << std::endl;
        }
        entities.push_back(std::move(bullet));
    }
    // std::cout << "Bullet spawned. Type: " << static_cast<int>(typeToSpawn) << std::endl; // Optional debug
}

void Game::spawnBossBullet(Boss* boss, int firePointIndex) {
    if (!boss || !boss->life) return;

    sf::Vector2f relativePos;
    switch(firePointIndex) {
        case 0: relativePos = boss->firePoint1; break;
        case 1: relativePos = boss->firePoint2; break;
        case 2: relativePos = boss->firePoint3; break;
        default: std::cerr << "Invalid boss fire point index: " << firePointIndex << std::endl; return;
    }

    sf::Vector2f startPos = boss->getAbsoluteFirePos(relativePos);

    Bullet::BulletType bossBulletType = Bullet::BulletType::Red; // Boss uses red bullets
    Animation* animPtr = &animBulletRed;
    float bulletAngle = 0;

    // Simple aim-at-player logic
    if (player && player->life) {
        sf::Vector2f direction = player->pos - startPos;
        bulletAngle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f + 90.f; // atan2 gives angle in radians, convert and adjust
    } else {
         bulletAngle = boss->angle + 180.f; // Fire straight 'down' relative to boss if player is dead/null
    }

    auto bullet = std::make_unique<Bullet>(bossBulletType);
    bullet->settings(*animPtr, startPos, bulletAngle);
    // TODO: Add bullet->isEnemy = true; flag and check in Player-Bullet collision
     if (bullet->type != Entity::Type::Bullet) { // Sanity check
          std::cerr << "Warning: Spawned boss bullet does not have Bullet type!" << std::endl;
     }
    entities.push_back(std::move(bullet));

    // Boss resets its own shoot timer after deciding to fire
    switch(firePointIndex) {
        case 0: boss->shootTimer1 = boss->shootCooldown * (1.0f + (rand()%20)/100.f); break;
        case 1: boss->shootTimer2 = boss->shootCooldown * (1.1f + (rand()%20)/100.f); break;
        case 2: boss->shootTimer3 = boss->shootCooldown * (1.2f + (rand()%20)/100.f); break;
    }
}

void Game::spawnPowerUp() {
    // Determine type
    int typeRoll = rand() % 3; // 0: Shield, 1: Weapon, 2: Speed (ExtraLife handled differently?)
    PowerUp::PowerUpType chosenType;
    switch(typeRoll) {
        case 0: chosenType = PowerUp::PowerUpType::Shield; break;
        case 1: chosenType = PowerUp::PowerUpType::Weapon; break;
        case 2: chosenType = PowerUp::PowerUpType::Speed; break;
        default: return; // Should not happen
    }

    // Create using raw pointer first to check validity after settings
    PowerUp* powerUp = new PowerUp(chosenType);

    // Calculate random position within bounds
    float margin = 50.f;
    sf::Vector2f pos(static_cast<float>(rand() % (WINDOW_WIDTH - (int)(2*margin)) + margin),
                      static_cast<float>(rand() % (WINDOW_HEIGHT - (int)(2*margin)) + margin));
    float radius = 15.f; // Default collision radius

    Animation dummyAnim; // PowerUp::settings loads its own texture/anim
    powerUp->settings(dummyAnim, pos, 0, radius);

    if (powerUp->life && (powerUp->type == Entity::Type::PowerUp)) { // Check if setup was successful and type is correct
        entities.push_back(std::unique_ptr<Entity>(powerUp)); // Transfer ownership to list
    } else {
        std::cerr << "Failed to spawn or configure PowerUp correctly. Deleting." << std::endl;
        delete powerUp; // Cleanup if settings failed or type is wrong
    }
}

void Game::spawnEffect(Animation& anim, sf::Vector2f pos) {
    auto effect = std::make_unique<Effect>();
    Animation animCopy = anim; // Effects need their own copy to manage state
    animCopy.reset();          // Ensure animation starts from frame 0
    animCopy.play();
    effect->settings(animCopy, pos); // Settings applies the animation and position
     if (effect->type != Entity::Type::Effect) { // Sanity check
        std::cerr << "Warning: Spawned effect does not have Effect type!" << std::endl;
     }
    entities.push_back(std::move(effect));
}

void Game::spawnBoss(int level) {
    if (currentBoss) {
        std::cerr << "Warning: Trying to spawn boss when one already exists." << std::endl;
        return;
    }

    std::cout << "Spawning Boss for Level " << level << std::endl;
    auto boss = std::make_unique<Boss>();
    currentBoss = boss.get(); // Assign raw pointer

    // TODO: Potentially choose boss type/animation based on level
    Animation* bossAnim = &animBoss1;

    boss->settings(*bossAnim, sf::Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.15f));
     if (boss->type != Entity::Type::Boss) { // Sanity check
        std::cerr << "Warning: Spawned boss does not have Boss type!" << std::endl;
     }
    entities.push_back(std::move(boss));

    // Music handled in loadLevel
}

void Game::triggerBossExplosion(sf::Vector2f bossPos) {
    // bossExplodeSound.play(); // TODO: Add sound buffer
    int numExplosions = 10;
    float radius = 60.f; // Spread radius for small explosions
    for (int i = 0; i < numExplosions; ++i) {
         float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.f * 3.14159f;
         float dist = (static_cast<float>(rand()) / RAND_MAX) * radius;
         sf::Vector2f offset(std::cos(angle) * dist, std::sin(angle) * dist);
         spawnEffect(animExplosionBoss, bossPos + offset); // Specific small boss explosions
    }
    // Add one larger one in the center
    spawnEffect(animExplosionAsteroid, bossPos); // Use large asteroid/general explosion
}

// --- Collision Detection ---
void Game::checkCollisions() {
    // Use iterators for safe removal if needed (though cleanupEntities is preferred)
    for (auto i = entities.begin(); i != entities.end(); ++i) {
        Entity* entityA = i->get();
        // Skip checks if entity is dead or has no collision radius
        if (!entityA->life || entityA->R <= 0) continue;

        for (auto j = std::next(i); j != entities.end(); ++j) {
            Entity* entityB = j->get();
            // Skip checks if second entity is dead or has no collision radius
            if (!entityB->life || entityB->R <= 0) continue;

            // Check distance
            if (isCollide(entityA, entityB)) {
                // Make pointers 'a' and 'b' point to the collided entities
                Entity* a = entityA;
                Entity* b = entityB;
                Entity::Type typeA = a->type;
                Entity::Type typeB = b->type;

                // Ensure typeA <= typeB for easier checking
                if (typeA > typeB) {
                    std::swap(a, b);
                    std::swap(typeA, typeB);
                }

                // --- Collision Pair Handling ---

                // Player(1) <-> Asteroid(2)
                if (typeA == Entity::Type::Player && typeB == Entity::Type::Asteroid) {
                    if (player && player->life) { // Check player still exists and alive
                         Asteroid* asteroid = static_cast<Asteroid*>(b);
                         if (player->shieldActive) {
                             player->shieldActive = false; player->shieldTimer = 0;
                             asteroid->life = false;
                             spawnEffect(animExplosionSmall, asteroid->pos);
                             explosionSoundAsteroid.play();
                         } else {
                             player->takeDamage();
                             explosionSoundPlayer.play();
                             spawnEffect(animExplosionPlayer, player->pos);
                             asteroid->life = false; // Asteroid also destroyed
                             // Check for respawn NEED after takeDamage
                             if (!player->life && player->lives > 0) {
                                 playerRespawnTimer = PLAYER_RESPAWN_DELAY;
                             }
                         }
                    }
                }
                // Player(1) <-> Bullet(3) (Assuming enemy bullets - Requires bullet flag)
                // else if (typeA == Entity::Type::Player && typeB == Entity::Type::Bullet) {
                //     Bullet* bullet = static_cast<Bullet*>(b);
                //     if (bullet->isEnemy && player && player->life) { /* Handle damage/respawn */ }
                // }

                // Player(1) <-> PowerUp(4) / PowerDown(5)
                else if (typeA == Entity::Type::Player && (typeB == Entity::Type::PowerUp || typeB == Entity::Type::PowerDown)) {
                     if (player && player->life) {
                         // PowerUp class handles distinguishing between Up/Down
                         player->applyPowerUp(static_cast<PowerUp*>(b));
                         b->life = false; // Consume item
                         powerupSound.play(); // Assuming sound is for good powerups only
                     }
                }
                 // Player(1) <-> Boss(7)
                else if (typeA == Entity::Type::Player && typeB == Entity::Type::Boss) {
                     if (player && player->life) {
                         if (player->shieldActive) {
                              player->shieldActive = false; player->shieldTimer = 0;
                              // static_cast<Boss*>(b)->takeDamage(2); // Minor damage to boss?
                         } else {
                             player->takeDamage(); // Player takes damage
                             explosionSoundPlayer.play();
                             spawnEffect(animExplosionPlayer, player->pos);
                             // static_cast<Boss*>(b)->takeDamage(5); // Maybe boss takes ram damage?
                             if (!player->life && player->lives > 0) {
                                 playerRespawnTimer = PLAYER_RESPAWN_DELAY;
                             }
                         }
                     }
                }
                // Player(1) <-> HazardMeteor(8)
                else if (typeA == Entity::Type::Player && typeB == Entity::Type::HazardMeteor) {
                     if (player && player->life) {
                         HazardMeteor* meteor = static_cast<HazardMeteor*>(b);
                         if (player->shieldActive) {
                              player->shieldActive = false; player->shieldTimer = 0;
                              meteor->life = false;
                              spawnEffect(animExplosionSmall, meteor->pos);
                              powerdownSound.play(); // Play sound even if shielded
                         } else {
                             player->slowTimer = 8.0f; // Apply slow effect
                             player->speedBoostTimer = 0.f; // Cancel speed boost
                             meteor->life = false;
                             spawnEffect(animExplosionSmall, meteor->pos);
                             powerdownSound.play();
                             // Hazard meteor ALSO damages player
                             player->takeDamage();
                             if (!player->life && player->lives > 0) {
                                 playerRespawnTimer = PLAYER_RESPAWN_DELAY;
                             }
                         }
                     }
                }

                // Asteroid(2) <-> Bullet(3)
                else if (typeA == Entity::Type::Asteroid && typeB == Entity::Type::Bullet) {
                     Asteroid* asteroid = static_cast<Asteroid*>(a);
                     Bullet* bullet = static_cast<Bullet*>(b);
                     // TODO: Ignore collision if bullet->isEnemy?
                     asteroid->life = false;
                     bullet->life = false;
                     if (player) player->addScore(asteroid->scoreValue);
                     explosionSoundAsteroid.play();
                     spawnEffect(animExplosionAsteroid, asteroid->pos);
                     // Spawn smaller asteroids
                     if (asteroid->getSize() == Asteroid::Size::Large) {
                         spawnAsteroid(Asteroid::Size::Medium, asteroid->pos);
                         spawnAsteroid(Asteroid::Size::Medium, asteroid->pos);
                     } else if (asteroid->getSize() == Asteroid::Size::Medium) {
                         spawnAsteroid(Asteroid::Size::Small, asteroid->pos);
                         spawnAsteroid(Asteroid::Size::Small, asteroid->pos);
                     }
                }

                // Bullet(3) <-> Boss(7)
                else if (typeA == Entity::Type::Bullet && typeB == Entity::Type::Boss) {
                     Bullet* bullet = static_cast<Bullet*>(a);
                     Boss* boss = static_cast<Boss*>(b);
                     // TODO: Ignore collision if !bullet->isEnemy? (Player bullet hits boss)
                     // if (!bullet->isEnemy) {
                         boss->takeDamage(bullet->damage);
                         bullet->life = false;
                         spawnEffect(animExplosionSmall, bullet->pos); // Hit spark
                         // bossHitSound.play();
                         if (!boss->life) {
                             triggerBossExplosion(boss->pos);
                             // Score/music handled in cleanupEntities
                         }
                     // }
                }
                // Bullet(3) <-> HazardMeteor(8)
                else if (typeA == Entity::Type::Bullet && typeB == Entity::Type::HazardMeteor) {
                     a->life = false; // Bullet
                     b->life = false; // Meteor
                     spawnEffect(animExplosionSmall, b->pos);
                     explosionSoundAsteroid.play(); // Reuse sound
                }

                // Other potential collisions (Asteroid-Asteroid, Asteroid-Hazard) ignored for now

            } // End if isCollide
        } // End inner loop (j)
    } // End outer loop (i)
}


void Game::cleanupEntities() {
    // Use std::list::remove_if for efficient removal
    entities.remove_if([this](const std::unique_ptr<Entity>& e) {
        // Điều kiện xóa cơ bản: life == false
        if (!e->life) {
            // --- XỬ LÝ ĐẶC BIỆT CHO PLAYER ---
            if (e->type == Entity::Type::Player) {
                // CHỈ xóa player nếu timer hồi sinh KHÔNG chạy (<= 0)
                // Nếu life == false NHƯNG timer > 0 nghĩa là đang chờ hồi sinh -> KHÔNG XÓA
                if (playerRespawnTimer <= 0) {
                    player = nullptr; // Xóa con trỏ raw khi unique_ptr bị xóa
                    std::cout << "Cleanup: Player removed (No respawn pending)." << std::endl;
                    return true; // Đánh dấu để xóa
                } else {
                    // Đang chờ hồi sinh, không làm gì cả, không xóa
                    // std::cout << "Cleanup: Player life is false, but respawn timer active. NOT removing." << std::endl; // Debug log (optional)
                    return false; // KHÔNG xóa player
                }
            }
            // --- KẾT THÚC XỬ LÝ PLAYER ---

            // Xử lý cho Boss (như cũ)
            else if (e->type == Entity::Type::Boss) {
                std::cout << "Cleanup: Boss entity removed." << std::endl;
                if (player) player->addScore(bossDefeatScoreBonus);
                currentBoss = nullptr;
                bossMusic.stop();
                if (currentState == State::Playing) backgroundMusic.play();
                return true; // Xóa Boss
            }
            // Các loại entity khác, nếu life == false thì xóa bình thường
            else {
                
                return true; // Đánh dấu để xóa các entity khác (bullet, asteroid, effect...)
            }
        }
        // Nếu life == true, không xóa
        return false;
    });
}

bool Game::checkLevelComplete() {
    // Level is complete if there's no active boss AND no asteroids left
    if (currentBoss && currentBoss->life) {
        return false; // Boss alive, not complete
    }

    for (const auto& entity : entities) {
        if (entity->life && entity->type == Entity::Type::Asteroid) {
            return false; // Found a live asteroid, not complete
        }
    }

    // No live boss and no live asteroids found
    return true;
}

void Game::nextLevel() {
    currentLevel++;
    std::cout << "Proceeding to Level " << currentLevel << std::endl;
    // State transition to LevelTransition happens in updatePlaying
}

void Game::showInstructions() {
    // Set text first
    messageText.setCharacterSize(24);
    messageText.setString(
        "Instructions:\n\n"
        "Arrow Keys Left/Right: Rotate Ship\n"
        "Arrow Key Up: Apply Thrust\n"
        "Spacebar: Fire Weapon\n"
        "Esc: Pause Game / Go Back\n\n"
        "Objective: Destroy all asteroids!\n\n"
        "Pickups:\n"
        "  Blue Shield: Temporary invincibility\n"
        "  Red Gears: Weapon upgrade\n"
        "  Green Flames: Speed boost\n\n"
        "Hazards:\n"
        "  Slow Meteors: Don't destroy ship, but cause slow down and damage!\n\n"
        "[Esc] Back to Main Menu"
    );
    // Center text based on its new content
    messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
    messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);

    // Set state AFTER text is fully configured
    // No need to call setState here if it was already called by handleInput
    // Check if the state is already Instructions
    if (currentState != State::Instructions) {
       setState(State::Instructions);
    }
    // If handleInput already called setState(Instructions), calling it again is redundant
    // but shouldn't cause a crash. The main fix is setting text *before* potential setState.
}

void Game::showStory(int level) {
    std::string story = "";
    bool showStoryText = true; // Assume story should be shown

    switch (level) {
        case 1: story = "Level 1:\nAn unexpected asteroid cluster has entered our sector.\nClear the area, rookie!"; break;
        case BOSS_LEVEL_INTERVAL: story = "WARNING:\nMassive energy signature detected!\nPrepare for contact!"; break;
        case 4: story = "Level 4:\nStrange, slowing meteors sighted.\nMaintain speed and clear the field."; break;
        case BOSS_LEVEL_INTERVAL * 2: story = "Hostile signature returning!\nIt seems... enhanced. Engage with extreme caution!"; break;
        // Add more cases
        default:
            showStoryText = false; // No specific story for this level
            break;
    }

    if (showStoryText) {
        messageText.setString(story); // Set the text first
        setState(State::Story);     // THEN set the state (which positions the text)
    } else {
        // No story, load level and go directly to playing
        loadLevel(currentLevel);
        setState(State::Playing);
    }
}

bool Game::isCollide(const Entity *a, const Entity *b) {
    // Basic circle collision check
    sf::Vector2f diff = b->pos - a->pos;
    float distSq = (diff.x * diff.x) + (diff.y * diff.y);
    float radiusSum = a->R + b->R;
    return distSq < (radiusSum * radiusSum);
}

// --- High Score ---
void Game::loadHighScore() {
    std::ifstream inputFile(HIGHSCORE_FILE);
    if (inputFile.is_open()) {
        if (!(inputFile >> highScore)) {
            std::cerr << "Warning: Could not read high score from " << HIGHSCORE_FILE << ". Using 0." << std::endl;
            highScore = 0;
        }
        inputFile.close();
        std::cout << "Loaded high score: " << highScore << std::endl;
    } else {
        std::cout << "High score file (" << HIGHSCORE_FILE << ") not found. Starting with 0." << std::endl;
        highScore = 0;
    }
}

void Game::saveHighScore() {
    std::ofstream outputFile(HIGHSCORE_FILE);
    if (outputFile.is_open()) {
        outputFile << highScore;
        outputFile.close();
        std::cout << "Saved new high score: " << highScore << std::endl;
    } else {
        std::cerr << "Error: Could not open " << HIGHSCORE_FILE << " for saving high score." << std::endl;
    }
}

// --- Ship Selection Helpers ---
void Game::cycleShipSelection() {
    int currentType = static_cast<int>(selectedShipType);
    currentType++;
    if (currentType > static_cast<int>(Player::ShipType::Heavy)) {
        currentType = static_cast<int>(Player::ShipType::Standard); // Wrap around
    }
    selectedShipType = static_cast<Player::ShipType>(currentType);
}

void Game::updateShipSelectionText() {
    std::string shipName;
    switch (selectedShipType) {
        case Player::ShipType::Standard: shipName = "Standard"; break;
        case Player::ShipType::Fast:     shipName = "Fast";     break;
        case Player::ShipType::Heavy:    shipName = "Heavy";    break;
        default:                         shipName = "Unknown";  break;
    }
    shipSelectionText.setString("Selected Ship: < " + shipName + " >");
}