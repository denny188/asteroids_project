#include "Game.h"
#include "ResourceManager.h"
#include "Animation.h"
#include "Asteroid.h"
#include "Bullet.h"
#include "PowerUp.h"
#include "HazardMeteor.h" // Include new meteor
#include "Effect.h"
#include "Boss.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

// --- Constants ---
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const float ASTEROID_SPAWN_RATE_BASE = 3.5f;
const float POWERUP_SPAWN_RATE_BASE = 12.0f;
const float HAZARD_METEOR_SPAWN_RATE = 15.0f; // How often slow meteors appear
const float PLAYER_RESPAWN_DELAY = 3.0f;
const float STORY_DISPLAY_DURATION = 4.0f; // How long story text shows
const int BOSS_LEVEL_INTERVAL = 3; // Boss appears every 3 levels in campaign

Game::Game() :
    window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Asteroids Deluxe"),
    currentState(State::MainMenu),
    currentMode(PlayMode::Campaign),
    resourceManager(ResourceManager::getInstance()),
    player(nullptr),
    currentBoss(nullptr), // Initialize boss pointer
    currentLevel(0),
    asteroidSpawnTimer(ASTEROID_SPAWN_RATE_BASE),
    powerUpSpawnTimer(POWERUP_SPAWN_RATE_BASE),
    hazardMeteorSpawnTimer(HAZARD_METEOR_SPAWN_RATE),
    playerRespawnTimer(0.f),
    bossDefeatScoreBonus(1000),
    storyDisplayTimer(0.f)
{
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    initialize();
}

Game::~Game() {
    // unique_ptr handles entity cleanup
    backgroundMusic.stop();
    bossMusic.stop();
}

void Game::initialize() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    loadResources(); // Load resources and create Animation objects
    setupUI();
    setState(State::MainMenu);
}

void Game::loadResources() {
    std::cout << "Loading resources and animations..." << std::endl;
    try {
        // --- Textures ---
        resourceManager.getTexture("spaceship.png"); // Placeholder for player
        resourceManager.getTexture("background.jpg");
        resourceManager.getTexture("rock.png");
        resourceManager.getTexture("rock_medium.png");
        resourceManager.getTexture("rock_small.png");
        resourceManager.getTexture("fire_blue.png");
        resourceManager.getTexture("fire_red.png");
        resourceManager.getTexture("fire_laser.png");
        resourceManager.getTexture("slow_powerdown.png"); // Hazard meteor texture
        resourceManager.getTexture("shield_powerup.png");
        resourceManager.getTexture("weapon_powerup.png");
        resourceManager.getTexture("speed_powerup.png");
        resourceManager.getTexture("boss1.png");
        // resourceManager.getTexture("boss2.png"); // If added
        resourceManager.getTexture("gameover.png");
        resourceManager.getTexture("explosions/type_A.png");
        resourceManager.getTexture("explosions/type_B.png");
        resourceManager.getTexture("explosions/type_C.png");
        resourceManager.getTexture("explosions/boss_explosion.png");

        // --- Create Animations ---
        // Asteroids
        animRockLarge = Animation(resourceManager.getTexture("rock.png"), 0, 0, 64, 64, 16, 0.2f);
        animRockMedium = Animation(resourceManager.getTexture("rock_medium.png"), 0, 0, 96, 96, 12, 0.25f);
        animRockSmall = Animation(resourceManager.getTexture("rock_small.png"), 0, 0, 64, 64, 16, 0.3f);
        // Bullets
        animBulletBlue = Animation(resourceManager.getTexture("fire_blue.png"), 0, 0, 32, 64, 16, 0.8f, false);
        animBulletRed = Animation(resourceManager.getTexture("fire_red.png"), 0, 0, 32, 64, 16, 0.9f, false);
        animBulletLaser = Animation(resourceManager.getTexture("fire_laser.png"), 0, 0, 64, 64, 18, 1.2f, false);
        // Hazard Meteor
        animHazardMeteor = Animation(resourceManager.getTexture("slow_powerdown.png"), 0, 0, 64, 64, 24, 0.3f, true);
        // Explosions
        // type_A.png: 1024x50, 20 frames => W ~ 51
        animExplosionSmall = Animation(resourceManager.getTexture("explosions/type_A.png"), 0, 0, 51, 50, 20, 0.6f, false);
        animExplosionPlayer = Animation(resourceManager.getTexture("explosions/type_B.png"), 0, 0, 192, 192, 64, 0.7f, false);
        animExplosionAsteroid = Animation(resourceManager.getTexture("explosions/type_C.png"), 0, 0, 256, 256, 48, 0.6f, false);
        animExplosionBoss = Animation(resourceManager.getTexture("explosions/boss_explosion.png"), 0, 0, 64, 64, 8, 0.5f, false);
        // Boss
        animBoss1 = Animation(resourceManager.getTexture("boss1.png"), 0, 0, 230, 336, 1, 0, false); // Static boss

        // --- Fonts ---
        if (!uiFont.loadFromFile("C:/Windows/Fonts/arial.ttf")) { // ADJUST PATH!
             throw std::runtime_error("Failed to load font: arial.ttf");
         }

        // --- Sounds ---
        shootSound.setBuffer(resourceManager.getSoundBuffer("shoot.wav"));
        explosionSoundAsteroid.setBuffer(resourceManager.getSoundBuffer("explosion_asteroid.wav"));
        explosionSoundPlayer.setBuffer(resourceManager.getSoundBuffer("explosion_player.wav"));
        powerupSound.setBuffer(resourceManager.getSoundBuffer("powerup_collect.wav"));
        powerdownSound.setBuffer(resourceManager.getSoundBuffer("powerdown.ogg")); // Assign slow hit sound
        // TODO: Load boss hit/explode sounds when available
        // bossHitSound.setBuffer(resourceManager.getSoundBuffer("boss_hit.wav"));
        // bossExplodeSound.setBuffer(resourceManager.getSoundBuffer("boss_explode.wav"));

        // --- Music ---
        if (!backgroundMusic.openFromFile("sounds/background_music.ogg")) throw std::runtime_error("Failed to load background music");
        backgroundMusic.setLoop(true); backgroundMusic.setVolume(30);
        if (!bossMusic.openFromFile("sounds/boss_theme.ogg")) throw std::runtime_error("Failed to load boss music");
        bossMusic.setLoop(true); bossMusic.setVolume(45);

    } catch (const std::exception& e) { // Catch std::exception for broader coverage
        std::cerr << "Error loading resources: " << e.what() << std::endl;
        window.close();
    }
    std::cout << "Resources loaded." << std::endl;
}

void Game::setupUI() {
    // Score, Lives, Level Text setup (same as before)
    scoreText.setFont(uiFont); scoreText.setCharacterSize(24); scoreText.setFillColor(sf::Color::White); scoreText.setPosition(10, 10);
    livesText.setFont(uiFont); livesText.setCharacterSize(24); livesText.setFillColor(sf::Color::White); livesText.setPosition(10, 40);
    levelText.setFont(uiFont); levelText.setCharacterSize(24); levelText.setFillColor(sf::Color::White); levelText.setPosition(window.getSize().x - 150.f, 10); // Adjusted position
    messageText.setFont(uiFont); messageText.setCharacterSize(40); messageText.setFillColor(sf::Color::White);

    // Setup Game Over Sprite
    try {
        gameOverSprite.setTexture(resourceManager.getTexture("gameover.png"));
        gameOverSprite.setOrigin(gameOverSprite.getLocalBounds().width / 2.f, gameOverSprite.getLocalBounds().height / 2.f);
        gameOverSprite.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f - 50); // Position slightly above center
    } catch (const std::runtime_error& e) {
         std::cerr << "Warning: Failed to load gameover.png texture. Game over screen will use text only." << std::endl;
    }

}

void Game::setState(State newState) {
    State oldState = currentState;
    currentState = newState;
    messageText.setString(""); // Clear messages

    // --- State Exit Actions ---
    if (oldState == State::Playing || oldState == State::Paused) {
         backgroundMusic.pause(); // Pause main music when leaving playing/paused state
         bossMusic.pause();     // Pause boss music too
    }

    // --- State Entry Actions ---
    switch (currentState) {
        case State::MainMenu:
            resetGame(true); // Full reset when going to main menu
            messageText.setString("ASTEROIDS DELUXE\n\n[P] Play Campaign\n[S] Play Survival\n[I] Instructions\n[Esc] Exit");
            messageText.setCharacterSize(40); // Reset size
            messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
            messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
            backgroundMusic.play(); // Start music
            break;
        case State::Instructions:
             showInstructions();
             break;
        case State::Story:
             // showStory() is called within loadLevel/startSurvival
             storyDisplayTimer = STORY_DISPLAY_DURATION; // Set timer
             messageText.setCharacterSize(30);
             // Text is set by showStory()
              messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
             messageText.setPosition(window.getSize().x / 2.f, window.getSize().y * 0.8f); // Display near bottom
             break;
        case State::Playing:
            if (oldState == State::Paused) {
                 // Resuming game
                 if (currentBoss) bossMusic.play(); else backgroundMusic.play();
            } else if (!player) { // Starting new game
                resetGame(true); // Full reset if coming from menu/game over
                if (currentMode == PlayMode::Campaign) {
                    currentLevel = 1;
                    setState(State::Story); // Show story before level 1
                } else {
                    currentLevel = 1;
                    startSurvival(); // Start survival directly (no story?)
                    backgroundMusic.play();
                }
            } else if (player && !player->life && player->lives > 0) { // Respawning
                 playerRespawnTimer = PLAYER_RESPAWN_DELAY; // Start respawn timer
                 if (currentBoss) bossMusic.play(); else backgroundMusic.play();
            } else { // Coming from Level Transition or Story
                 if (currentBoss) bossMusic.play(); else backgroundMusic.play();
            }
            break;
        case State::LevelTransition:
             messageText.setString("Level " + std::to_string(currentLevel) + " Complete!");
             // Add score bonus maybe?
             messageText.setCharacterSize(40);
             messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
             messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
             clock.restart(); // Use clock for transition delay
            break;
        case State::GameOver:
             // messageText set below (using score)
             messageText.setCharacterSize(30);
             messageText.setString("\n\nFinal Score: " + std::to_string(player ? player->score : 0) + "\n\n[R] Retry\n[M] Main Menu");
             messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
             // Position text below the game over sprite
             messageText.setPosition(gameOverSprite.getPosition().x, gameOverSprite.getPosition().y + gameOverSprite.getGlobalBounds().height / 2.f + 50.f);
            break;
        case State::Paused:
             messageText.setString("PAUSED\n\n[Esc] Resume\n[M] Main Menu");
             messageText.setCharacterSize(40);
             messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
             messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
             break;
    }
}


void Game::run() { /* same as before */
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Prevent spiral of death
        handleInput();
        update(dt);
        render();
    }
}

void Game::handleInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        // Global inputs
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                if (currentState == State::Playing) setState(State::Paused);
                else if (currentState == State::Paused) setState(State::Playing);
                else if (currentState == State::Instructions) setState(State::MainMenu);
                else if (currentState == State::Story) { /* Maybe skip story? Or handled by timer */ }
                else if (currentState == State::MainMenu) window.close();
            }
            if (event.key.code == sf::Keyboard::M) {
                 if (currentState == State::Paused || currentState == State::GameOver) {
                    setState(State::MainMenu);
                 }
            }
        }

        // State-specific inputs
        switch (currentState) {
            case State::MainMenu:
                 if (event.type == sf::Event::KeyPressed) { /* P, S, I keys */
                    if (event.key.code == sf::Keyboard::P) { currentMode = PlayMode::Campaign; setState(State::Playing); }
                    else if (event.key.code == sf::Keyboard::S) { currentMode = PlayMode::Survival; setState(State::Playing); }
                    else if (event.key.code == sf::Keyboard::I) { setState(State::Instructions); }
                 }
                 break;
             case State::Instructions: break; // Esc handled globally
             case State::Story: break; // Waits for timer or skip key? (Add skip later if needed)
             case State::Playing:
                 if (event.type == sf::Event::KeyPressed) {
                     if (event.key.code == sf::Keyboard::Space && player && player->life) {
                         player->shoot(); // Player signals intent, Game checks cooldown
                         if(player->shootTimer <= 0) { // Check if allowed to shoot now
                             spawnBullet(); // Spawn if cooldown ready
                         }
                     }
                 }
                 break;
             case State::LevelTransition: break; // Waits for timer
             case State::Paused: break; // Esc/M handled globally
             case State::GameOver:
                  if (event.type == sf::Event::KeyPressed) {
                      if (event.key.code == sf::Keyboard::R) {
                           setState(State::Playing); // Resets based on mode in setState
                      }
                  }
                  break;
        }
    }
}

void Game::update(float dt) {
    switch (currentState) {
        case State::MainMenu: updateMainMenu(dt); break;
        case State::Playing: updatePlaying(dt); break;
        case State::LevelTransition: updateLevelTransition(dt); break;
        case State::GameOver: updateGameOver(dt); break;
        case State::Story: updateStory(dt); break;
        // No update needed for Instructions, Paused
        case State::Instructions: break;
        case State::Paused: break;
    }
}

void Game::updateMainMenu(float dt) { /* ... */ }
void Game::updateGameOver(float dt) { /* ... */ }

void Game::updateStory(float dt) {
    storyDisplayTimer -= dt;
    if (storyDisplayTimer <= 0) {
         // Story finished, load level or start survival
         if (currentMode == PlayMode::Campaign) {
             loadLevel(currentLevel); // Load the actual level now
             setState(State::Playing);
         } else { // Survival doesn't use story currently
              startSurvival();
              setState(State::Playing);
         }
    }
}

void Game::updatePlaying(float dt) {
    // --- Respawn Player Logic ---
    if (playerRespawnTimer > 0) { // Check timer riêng lẻ
        playerRespawnTimer -= dt;
        if (playerRespawnTimer <= 0) {
            if (player && player->lives > 0) { // Player đã tồn tại (chỉ bị tạm ngưng) và còn mạng
                std::cout << "Respawn timer ended. Reviving player..." << std::endl;
                player->reset(); // Đặt lại trạng thái (quan trọng là life=true)
                player->pos = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
                player->velocity = sf::Vector2f(0,0);
                player->life = true; // Đảm bảo life = true
                player->shieldActive = true; // Respawn shield
                player->shieldTimer = 2.0f;
            } else if (!player && /* cần biến lưu số mạng cũ > 0 */ true) { // Player đã bị xóa hoàn toàn (không nên xảy ra nếu takeDamage đúng)
                 std::cout << "Respawn timer ended, player pointer was null. Spawning new player..." << std::endl;
                 spawnPlayer();
                 if(player) {
                    player->shieldActive = true;
                    player->shieldTimer = 2.0f;
                 }
            } else { // Timer hết nhưng hết mạng -> Game Over
                std::cout << "Respawn timer ended but no lives left. Triggering Game Over." << std::endl;
                setState(State::GameOver);
                return; // Dừng updatePlaying
            }
        }
    } // --- Hết Respawn Logic ---


    // --- Spawning ---
    // Spawn Asteroids (chỉ khi không có boss)
    if (!currentBoss) {
        asteroidSpawnTimer -= dt;
        if (asteroidSpawnTimer <= 0) {
            // Spawn ngẫu nhiên kích thước
            int sizeRoll = rand() % 3;
            Asteroid::Size spawnSize;
            switch (sizeRoll) {
                case 0: spawnSize = Asteroid::Size::Large; break;
                case 1: spawnSize = Asteroid::Size::Medium; break;
                case 2: spawnSize = Asteroid::Size::Small; break;
            }
            spawnAsteroid(spawnSize);
            asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE / (1.0f + currentLevel * 0.05f);
        }
    }

    // Spawn Hazard Meteors
    hazardMeteorSpawnTimer -= dt;
    if (hazardMeteorSpawnTimer <= 0) {
        spawnHazardMeteor();
        hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE * (0.8f + static_cast<float>(rand() % 40) / 100.f);
    }

    // Spawn Power-ups
    powerUpSpawnTimer -= dt;
    if (powerUpSpawnTimer <= 0) {
        spawnPowerUp();
        powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE * (0.9f + static_cast<float>(rand() % 20) / 100.f);
    }
    // --- Hết Spawning ---


    // --- Update Entities ---
    for (auto it = entities.begin(); it != entities.end(); /* no increment */) {
        Entity* e = it->get();

        // Chỉ update nếu entity đang sống HOẶC nếu là player đang chờ hồi sinh (player->life=false nhưng playerRespawnTimer>0)
        // Thực ra player chờ hồi sinh không cần update logic di chuyển/vẽ, chỉ cần timer chạy
        if (e->life) {
             e->update(dt, window.getSize());

            // Boss specific shooting logic trigger
            if (e->type == Entity::Type::Boss) {
                Boss* boss = static_cast<Boss*>(e);
                if (boss->shootTimer1 <= 0) { spawnBossBullet(boss, 0); boss->shootTimer1 = boss->shootCooldown * (1.0f + (rand()%20)/100.f); }
                if (boss->shootTimer2 <= 0 && boss->currentPhase > 0) { spawnBossBullet(boss, 1); boss->shootTimer2 = boss->shootCooldown * (1.1f + (rand()%20)/100.f); }
                if (boss->shootTimer3 <= 0 && boss->currentPhase > 1) { spawnBossBullet(boss, 2); boss->shootTimer3 = boss->shootCooldown * (1.2f + (rand()%20)/100.f); }
            }
             ++it; // Chỉ tăng iterator nếu entity còn sống và đã update
        } else {
             // Nếu entity không sống, chuẩn bị xóa nó ở bước cleanupEntities sau
             // Không cần update nó nữa
             ++it; // Vẫn phải tăng iterator để duyệt tiếp
        }
    }
    // --- Hết Update Entities ---


    // --- Collisions and Cleanup ---
    checkCollisions(); // Check va chạm giữa các entity còn sống
    cleanupEntities(); // Xóa các entity có life = false
    // --- Hết Collisions and Cleanup ---


    // --- Update UI ---
    if (player) { // Kiểm tra xem player còn tồn tại không (có thể vừa bị xóa trong cleanup)
        scoreText.setString("Score: " + std::to_string(player->score));
        livesText.setString("Lives: " + std::to_string(player->lives));
    } else if (playerRespawnTimer <= 0 && currentState != State::GameOver) {
        // Nếu player null, không trong thời gian chờ, và chưa game over -> chắc chắn là game over
        setState(State::GameOver);
        return;
    } else {
        // Player đang chờ hồi sinh hoặc đã game over
        scoreText.setString("Score: ..."); // Hoặc giữ nguyên điểm cũ?
        livesText.setString("Lives: 0");
    }
    levelText.setString(((currentMode == PlayMode::Campaign) ? "Level: " : "Wave: ") + std::to_string(currentLevel));
    // --- Hết Update UI ---


    // --- Check Level Completion (Campaign Mode) ---
    if (currentMode == PlayMode::Campaign && checkLevelComplete()) {
         // Đảm bảo player còn sống hoặc không cần chờ hồi sinh để qua màn
         if (!currentBoss && (!player || player->life || playerRespawnTimer <=0 )) {
             nextLevel();
             setState(State::LevelTransition);
         }
    }
    // --- Hết Check Level Completion ---
}

void Game::updateLevelTransition(float dt) {
     if (clock.getElapsedTime().asSeconds() > 2.0f) { // Shorter transition
         setState(State::Story); // Show story before loading next level
     }
}

bool Game::checkLevelComplete() {
    if (currentBoss && currentBoss->life) {
        return false; // Boss is active, level not complete
    }
    // Check if any asteroids remain
    for (const auto& entity : entities) {
        if (entity->type == Entity::Type::Asteroid) {
            return false;
        }
    }
    return true; // No asteroids and no living boss
}

void Game::nextLevel() {
    currentLevel++;
}

void Game::cleanupEntities() {
    entities.remove_if([this](const std::unique_ptr<Entity>& e) {
        if (!e->life) {
            if (e->type == Entity::Type::Player) {
                player = nullptr; // Player unique_ptr is removed, nullify raw pointer
            } else if (e->type == Entity::Type::Boss) {
                 currentBoss = nullptr; // Boss unique_ptr is removed
                 if (player) player->addScore(bossDefeatScoreBonus); // Score for defeating boss
                 // Switch back to background music?
                 bossMusic.stop();
                 backgroundMusic.play();
                 // Trigger explosion effect handled in checkCollisions or here?
            }
            return true; // Remove entity
        }
        return false;
    });
}


void Game::checkCollisions() {
    for (auto i = entities.begin(); i != entities.end(); ++i) {
        if (!(*i)->life || (*i)->R <= 0) continue;
        for (auto j = std::next(i); j != entities.end(); ++j) {
            if (!(*j)->life || (*j)->R <= 0) continue;

            Entity* a = i->get();
            Entity* b = j->get();

            if (isCollide(a, b)) {
                Entity::Type typeA = a->type;
                Entity::Type typeB = b->type;

                // Simplify check order
                if (typeA > typeB) { // Ensure typeA is always <= typeB for fewer checks
                    std::swap(a, b);
                    std::swap(typeA, typeB);
                }

                // --- Player <-> Asteroid ---
                if (typeA == Entity::Type::Player && typeB == Entity::Type::Asteroid) {
                    if (player && player->life && !player->shieldActive) {
                        std::cout << "Player collision! Lives before: " << player->lives << std::endl; // DEBUG
                        player->takeDamage();
                        explosionSoundPlayer.play();
                        spawnEffect(animExplosionPlayer, player->pos);
                        //a->life = false; // Player dies (will be removed, pointer nulled)
                        b->life = false; // Asteroid destroyed
                        if(player && !player->life && player->lives > 0) {
                            playerRespawnTimer = PLAYER_RESPAWN_DELAY; // Set timer if lives left
                            std::cout << "Player died, setting respawn timer. Lives left: " << player->lives << std::endl; // DEBUG
                        }
                        else if (player && player->life && player->shieldActive) {
                            player->shieldActive = false; player->shieldTimer = 0; // Shield breaks
                            b->life = false; // Asteroid destroyed
                            spawnEffect(animExplosionSmall, b->pos); // Small explosion for shield hit
                            explosionSoundAsteroid.play(); // Asteroid explosion sound
                        }
                        else if (player && !player->life && player->lives <= 0) {
                            std::cout << "Player died, no lives left. Game Over soon." << std::endl; // DEBUG
                        }
                    }
                }
                // --- Player <-> Bullet (Enemy Bullet - TODO) ---
                // else if (typeA == Entity::Type::Player && typeB == Entity::Type::Bullet && static_cast<Bullet*>(b)->isEnemyBullet) { ... }

                // --- Player <-> PowerUp/Down ---
                else if (typeA == Entity::Type::Player && (typeB == Entity::Type::PowerUp || typeB == Entity::Type::PowerDown)) {
                    if (player && player->life) {
                         player->applyPowerUp(static_cast<PowerUp*>(b));
                         b->life = false; // Collect powerup
                         powerupSound.play();
                    }
                }
                // --- Player <-> HazardMeteor ---
                else if (typeA == Entity::Type::Player && typeB == Entity::Type::HazardMeteor) {
                    if (player && player->life && !player->shieldActive) {
                        // Apply slow effect directly
                        player->slowTimer = 8.0f; // Duration of slow effect
                        player->speedBoostTimer = 0.f; // Cancel speed boost
                        b->life = false; // Destroy the meteor
                        spawnEffect(animExplosionSmall, b->pos); // Small effect on hit
                        powerdownSound.play(); // Play slow sound
                        player->takeDamage(); // Player takes damage (trigger respawn logic)
                    } else if (player && player->life && player->shieldActive) {
                         player->shieldActive = false; player->shieldTimer = 0; // Shield breaks
                         b->life = false; // Destroy meteor anyway
                         spawnEffect(animExplosionSmall, b->pos);
                         powerdownSound.play(); // Maybe play sound even with shield
                    }
               }
                 // --- Player <-> Boss ---
                else if (typeA == Entity::Type::Player && typeB == Entity::Type::Boss) {
                    if (player && player->life && !player->shieldActive) {
                        player->takeDamage(); // Player takes damage from collision
                        explosionSoundPlayer.play();
                        spawnEffect(animExplosionPlayer, player->pos);
                        a->life = false; // Player dies
                        if(player->lives > 0) playerRespawnTimer = PLAYER_RESPAWN_DELAY;
                        // Boss might take minor damage from ramming?
                        // static_cast<Boss*>(b)->takeDamage(5);
                    } else if (player && player->life && player->shieldActive) {
                        player->shieldActive = false; player->shieldTimer = 0; // Shield breaks
                        // Boss takes maybe minor damage?
                        // static_cast<Boss*>(b)->takeDamage(2);
                    }
                }

                // --- Bullet <-> Asteroid ---
                else if (typeA == Entity::Type::Bullet && typeB == Entity::Type::Asteroid) {
                    Bullet* bullet = static_cast<Bullet*>(a);
                    Asteroid* asteroid = static_cast<Asteroid*>(b);
                    asteroid->life = false;
                    bullet->life = false;
                    if (player) player->addScore(asteroid->scoreValue);
                    explosionSoundAsteroid.play();
                    spawnEffect(animExplosionAsteroid, asteroid->pos); // Use correct explosion anim
                    if (asteroid->getSize() == Asteroid::Size::Large) { /* spawn medium */
                        spawnAsteroid(Asteroid::Size::Medium, asteroid->pos); spawnAsteroid(Asteroid::Size::Medium, asteroid->pos);
                    } else if (asteroid->getSize() == Asteroid::Size::Medium) { /* spawn small */
                         spawnAsteroid(Asteroid::Size::Small, asteroid->pos); spawnAsteroid(Asteroid::Size::Small, asteroid->pos);
                    }
                }
                 // --- Bullet <-> HazardMeteor ---
                 else if (typeA == Entity::Type::Bullet && typeB == Entity::Type::HazardMeteor) {
                     a->life = false; // Bullet destroyed
                     b->life = false; // Meteor destroyed
                     spawnEffect(animExplosionSmall, b->pos); // Small explosion
                     explosionSoundAsteroid.play(); // Use asteroid sound?
                 }
                  // --- Bullet <-> Boss ---
                  else if (typeA == Entity::Type::Bullet && typeB == Entity::Type::Boss) {
                       Bullet* bullet = static_cast<Bullet*>(a);
                       Boss* boss = static_cast<Boss*>(b);
                       boss->takeDamage(bullet->damage); // Boss takes damage from bullet
                       bullet->life = false; // Destroy bullet
                       spawnEffect(animExplosionSmall, bullet->pos); // Small hit effect on boss
                       // bossHitSound.play();
                       // Check if boss died
                       if (!boss->life) {
                           triggerBossExplosion(boss->pos);
                           // Boss removal and score handled in cleanupEntities
                       }
                  }

                // --- Asteroid <-> Asteroid --- (Optional: make them bounce?)
                // else if (typeA == Entity::Type::Asteroid && typeB == Entity::Type::Asteroid) { /* Bounce logic */ }

                // --- Asteroid <-> HazardMeteor --- (Optional: bounce?)
                 // else if (typeA == Entity::Type::Asteroid && typeB == Entity::Type::HazardMeteor) { /* Bounce logic */ }
            }
        }
    }
}


void Game::render() {
    window.clear(sf::Color::Black);
    // Draw background
    try { /* same as before */
        sf::Sprite backgroundSprite(resourceManager.getTexture("background.jpg"));
        backgroundSprite.setScale(static_cast<float>(window.getSize().x) / backgroundSprite.getLocalBounds().width, static_cast<float>(window.getSize().y) / backgroundSprite.getLocalBounds().height);
        window.draw(backgroundSprite);
    } catch (const std::runtime_error& e) { // <<<--- THÊM CATCH CỤ THỂ
        std::cerr << "Error rendering background: " << e.what() << std::endl;
        // Không vẽ background nếu có lỗi
    }

    // Draw entities (Effects first, then others, Player last with overlays)
    for (const auto& entity : entities) { if(entity->type == Entity::Type::Effect) entity->draw(window); }
    for (const auto& entity : entities) { if(entity->type != Entity::Type::Effect && entity.get() != player) entity->draw(window); }
    if (player && player->life) player->draw(window); // Player draw now handles overlays


    // Draw UI / Messages based on state
    switch (currentState) {
        case State::MainMenu: renderMainMenu(); break;
        case State::Instructions: renderInstructions(); break;
        case State::Story: renderStory(); break;
        case State::Playing: renderPlaying(); break;
        case State::LevelTransition: renderLevelTransition(); break;
        case State::GameOver: renderGameOver(); break;
        case State::Paused: renderPaused(); break; // Changed from renderPlaying
    }
    window.display();
}

void Game::renderMainMenu() { window.draw(messageText); }
void Game::renderInstructions() { window.draw(messageText); }
void Game::renderStory() { window.draw(messageText); } // Just show the story text

void Game::renderPlaying() {
    window.draw(scoreText);
    window.draw(livesText);
    window.draw(levelText);
    // Optional: Draw boss health bar if boss exists
    if (currentBoss && currentBoss->life) {
        float barWidth = 300.f;
        float barHeight = 15.f;
        float healthPercent = static_cast<float>(currentBoss->health) / currentBoss->maxHealth;
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
void Game::renderLevelTransition() { renderPlaying(); window.draw(messageText); } // Show stats during transition

void Game::renderGameOver() {
    if (gameOverSprite.getTexture()) { // Draw sprite if loaded
        window.draw(gameOverSprite);
    }
    window.draw(messageText); // Draw text below sprite
}

void Game::renderPaused() {
     renderPlaying(); // Draw the paused game state
     // Draw semi-transparent overlay
      sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
      overlay.setFillColor(sf::Color(0, 0, 0, 150));
      window.draw(overlay);
     // Draw Pause message
     window.draw(messageText);
}


void Game::loadLevel(int levelNum) {
    std::cout << "Loading Level: " << levelNum << std::endl;
    resetGame(false); // Partial reset (keep score/lives)

    spawnPlayer(); // Ensure player exists

    int numAsteroids = 4 + levelNum; // Fewer base, scales slower
    int numHazards = levelNum / 2; // Add slow meteors based on level

    for (int i = 0; i < numAsteroids; ++i) {
        spawnAsteroid(Asteroid::Size::Large); // Start with large ones
    }
     for (int i = 0; i < numHazards; ++i) {
         spawnHazardMeteor();
     }

    // Spawn Boss?
    if (currentMode == PlayMode::Campaign && levelNum > 0 && levelNum % BOSS_LEVEL_INTERVAL == 0) {
        spawnBoss(levelNum);
        bossMusic.play(); // Start boss music
    } else {
         backgroundMusic.play(); // Start normal music
    }

    asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE;
    powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE;
    hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE;
    playerRespawnTimer = 0.f;

    // Story is shown before this via Story state
}

void Game::startSurvival() {
    std::cout << "Starting Survival Mode" << std::endl;
    resetGame(true); // Full reset for survival
    currentLevel = 1;

    spawnPlayer();

    for (int i = 0; i < 3; ++i) { // Start with fewer asteroids
        spawnAsteroid(Asteroid::Size::Large);
    }

    asteroidSpawnTimer = ASTEROID_SPAWN_RATE_BASE;
    powerUpSpawnTimer = POWERUP_SPAWN_RATE_BASE;
    hazardMeteorSpawnTimer = HAZARD_METEOR_SPAWN_RATE;
    playerRespawnTimer = 0.f;
    backgroundMusic.play();
}

void Game::spawnPlayer() {
     if (player) return; // Already exists

     auto newPlayer = std::make_unique<Player>();
     player = newPlayer.get();
     Animation dummyAnim; // Tạo một animation trống, không dùng đến
     player->settings(dummyAnim, // Tham số này sẽ bị bỏ qua trong Player::settings đã sửa
                      sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f));
     // Player score/lives được xử lý bởi resetGame

     entities.push_back(std::move(newPlayer));
     std::cout << "Player spawned." << std::endl; // Debug log
}


void Game::resetGame(bool fullReset) {
    entities.clear(); // Always clear entities
    player = nullptr;
    currentBoss = nullptr;
    if (fullReset) {
        currentLevel = 1;
        // Player score/lives reset when player is recreated in spawnPlayer/settings
    } else {
        // Keep currentLevel
        // Keep player score/lives (they persist in the player object if it exists)
        spawnPlayer(); // Ensure player is added back after clearing entities
        if (player) {
            player->reset(); // Reset position, velocity, effects, but keep score/lives
            player->pos = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f);
        }
    }
}

void Game::spawnAsteroid(Asteroid::Size size, sf::Vector2f pos) {
    auto asteroid = std::make_unique<Asteroid>(size);
    Animation* animPtr = nullptr;
    float radius = 25.f;

    switch(size) {
        case Asteroid::Size::Large: animPtr = &animRockLarge; radius = 25.f; break;
        case Asteroid::Size::Medium: animPtr = &animRockMedium; radius = 15.f; break;
        case Asteroid::Size::Small: animPtr = &animRockSmall; radius = 8.f; break;
    }

    if (!animPtr) {
        std::cerr << "Error: Could not find animation for asteroid size!" << std::endl;
        return;
    }

     if (pos.x == -100 && pos.y == -100) { /* Calculate spawn position same as before */
         int edge = rand() % 4;
         switch(edge) {
             case 0: pos = sf::Vector2f(static_cast<float>(rand() % WINDOW_WIDTH), -radius); break;
             case 1: pos = sf::Vector2f(static_cast<float>(WINDOW_WIDTH + radius), static_cast<float>(rand() % WINDOW_HEIGHT)); break;
             case 2: pos = sf::Vector2f(static_cast<float>(rand() % WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT + radius)); break;
             case 3: pos = sf::Vector2f(-radius, static_cast<float>(rand() % WINDOW_HEIGHT)); break;
         }
     }
     asteroid->settings(*animPtr, pos, static_cast<float>(rand() % 360), radius);
     entities.push_back(std::move(asteroid));
}

void Game::spawnHazardMeteor() {
     auto meteor = std::make_unique<HazardMeteor>();
      sf::Vector2f pos;
      float radius = 20.f; // Meteor radius
      int edge = rand() % 4;
      switch(edge) { /* Calculate spawn pos same as asteroid */
         case 0: pos = sf::Vector2f(static_cast<float>(rand() % WINDOW_WIDTH), -radius); break;
         case 1: pos = sf::Vector2f(static_cast<float>(WINDOW_WIDTH + radius), static_cast<float>(rand() % WINDOW_HEIGHT)); break;
         case 2: pos = sf::Vector2f(static_cast<float>(rand() % WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT + radius)); break;
         case 3: pos = sf::Vector2f(-radius, static_cast<float>(rand() % WINDOW_HEIGHT)); break;
      }
      meteor->settings(animHazardMeteor, pos, static_cast<float>(rand() % 360), radius);
      entities.push_back(std::move(meteor));
}


void Game::spawnBullet() {
    // *** THÊM KIỂM TRA COOLDOWN Ở ĐẦU HÀM ***
    if (!player || !player->life || player->shootTimer > 0) {
        std::cout << "Cannot shoot: Player null, dead, or cooldown active (" << (player ? player->shootTimer : -1.0f) << ")" << std::endl; // DEBUG
        return;
    }

    // Nếu qua được kiểm tra cooldown:
    player->shootTimer = player->shootCooldown; // <<<---- RESET TIMER Ở ĐÂY
    shootSound.play(); // <<<---- CHƠI ÂM THANH Ở ĐÂY

    Bullet::BulletType typeToSpawn = player->currentWeaponType;
    Animation* animPtr = nullptr;
    int bulletsToSpawn = 1;
    float spreadAngle = 15.f; // Degrees

    switch(typeToSpawn) {
        case Bullet::BulletType::Standard: animPtr = &animBulletBlue; bulletsToSpawn=1; break;
        case Bullet::BulletType::Laser: animPtr = &animBulletLaser; bulletsToSpawn=1; break;
        case Bullet::BulletType::Red: animPtr = &animBulletRed; bulletsToSpawn=1; break;
        case Bullet::BulletType::Spread: animPtr = &animBulletBlue; bulletsToSpawn=3; break; // Use blue anim for spread
    }

    if (!animPtr) return;

    for (int i = 0; i < bulletsToSpawn; ++i) {
         auto bullet = std::make_unique<Bullet>(typeToSpawn);
         float shotAngle = player->angle;
         if (bulletsToSpawn > 1) {
             shotAngle += (i - (bulletsToSpawn - 1) / 2.0f) * spreadAngle;
         }
         float offset = player->R;
         float angleRad = (player->angle - 90) * 3.14159f / 180.f; // Use player angle for offset
         sf::Vector2f startPos = player->pos + sf::Vector2f(std::cos(angleRad) * offset, std::sin(angleRad) * offset);

         bullet->settings(*animPtr, startPos, shotAngle); // Pass the determined animation
         entities.push_back(std::move(bullet));
    }
    std::cout << "Bullet spawned. Type: " << static_cast<int>(typeToSpawn) << std::endl; // DEBUG
}

void Game::spawnBossBullet(Boss* boss, int firePointIndex) {
    if (!boss || !boss->life) return;

    sf::Vector2f relativePos;
    switch(firePointIndex) {
        case 0: relativePos = boss->firePoint1; break;
        case 1: relativePos = boss->firePoint2; break;
        case 2: relativePos = boss->firePoint3; break;
        default: return; // Invalid index
    }

    sf::Vector2f startPos = boss->getAbsoluteFirePos(relativePos);

    // Boss bullet type and angle (e.g., aimed at player or fixed pattern)
    Bullet::BulletType bossBulletType = Bullet::BulletType::Red; // Example: Boss uses red bullets
    Animation* animPtr = &animBulletRed;
    float bulletAngle = 0;

    // Aim at player example
    if (player && player->life) {
        sf::Vector2f direction = player->pos - startPos;
        bulletAngle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f + 90.f; // Calculate angle towards player
    } else {
         bulletAngle = boss->angle + 180.f; // Fire straight down if player dead
    }


    auto bullet = std::make_unique<Bullet>(bossBulletType);
    bullet->settings(*animPtr, startPos, bulletAngle);
    // bullet->isEnemyBullet = true; // TODO: Add a flag to Bullet class to distinguish enemy bullets
    entities.push_back(std::move(bullet));
}


void Game::spawnPowerUp() { /* Same logic as before, but uses loaded animations */
    PowerUp* powerUp = nullptr;
    Animation* animPtr = nullptr; // Use pointers to loaded animations
    float radius = 12.f;
    sf::Vector2f pos(static_cast<float>(rand() % (WINDOW_WIDTH - 100) + 50),
                      static_cast<float>(rand() % (WINDOW_HEIGHT - 100) + 50));
    int typeRoll = rand() % 4; // Shield, Weapon, Speed, (Extra Life maybe later)

    PowerUp::PowerUpType chosenType;
     // TODO: No collectible power-downs for now, use HazardMeteor

    switch(typeRoll) {
        case 0: chosenType = PowerUp::PowerUpType::Shield; /* animPtr = &animShieldPU; */ break;
        case 1: chosenType = PowerUp::PowerUpType::Weapon; /* animPtr = &animWeaponPU; */ break;
        case 2: chosenType = PowerUp::PowerUpType::Speed;  /* animPtr = &animSpeedPU; */ break;
        // case 3: chosenType = PowerUp::PowerUpType::ExtraLife; /* animPtr = &animExtraLifePU; */ break;
        default: return; // Should not happen
    }

    // Need to load specific powerup anims in loadResources first and assign pointers here
    // For now, just create the PowerUp object type
    powerUp = new PowerUp(chosenType);

    // The settings method in PowerUp now tries to load the texture based on type
    // We pass a dummy animation here, PowerUp::settings will load the real one
     Animation dummyAnim; // Create a default/dummy animation
     powerUp->settings(dummyAnim, pos, 0, radius);

     if (powerUp->life) { // Check if settings succeeded (e.g., texture loaded)
        entities.push_back(std::unique_ptr<Entity>(powerUp)); // Transfer ownership
     } else {
         delete powerUp; // Settings failed, cleanup
     }
}

void Game::spawnEffect(Animation& anim, sf::Vector2f pos) {
    // This function now correctly takes a reference to a pre-loaded Animation object
    auto effect = std::make_unique<Effect>();
    // Need to make a copy of the animation so each effect instance is independent
    Animation animCopy = anim; // Make a copy
    effect->settings(animCopy, pos);
    entities.push_back(std::move(effect));
}

void Game::spawnBoss(int level) {
     if (currentBoss) return; // Only one boss at a time

     std::cout << "Spawning Boss for Level " << level << std::endl;
     auto boss = std::make_unique<Boss>();
     // TODO: Choose boss type based on level? For now, always Boss1
     currentBoss = boss.get(); // Store raw pointer
     boss->settings(animBoss1, sf::Vector2f(window.getSize().x / 2.f, window.getSize().y * 0.15f)); // Position near top center
     entities.push_back(std::move(boss));

     // Stop background music, start boss music
     backgroundMusic.stop();
     bossMusic.play();
}

void Game::triggerBossExplosion(sf::Vector2f bossPos) {
    // bossExplodeSound.play();
     // Spawn multiple small explosions around the boss area
     int numExplosions = 10;
     float radius = 50.f; // Radius to spawn explosions within
     for (int i = 0; i < numExplosions; ++i) {
          float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.f * 3.14159f;
          float dist = (static_cast<float>(rand()) / RAND_MAX) * radius;
          sf::Vector2f offset(std::cos(angle) * dist, std::sin(angle) * dist);
          spawnEffect(animExplosionBoss, bossPos + offset); // Use the specific boss explosion anim
     }
     // Add one big one in the center?
     spawnEffect(animExplosionAsteroid, bossPos); // Use large asteroid explosion anim
}


void Game::showInstructions() { /* Same as before */
     currentState = State::Instructions;
     messageText.setCharacterSize(24);
     messageText.setString(/* Instruction Text */
         "Instructions:\n\nArrow Keys Left/Right: Rotate\nArrow Key Up: Thrust\nSpacebar: Shoot\nEsc: Pause / Back\n\nDestroy Asteroids, Avoid Collisions!\nBlue=Shield, Red=Weapon, Green=Speed\nWatch out for Slow Meteors!\n\n[Esc] Back"
         );
     messageText.setOrigin(messageText.getLocalBounds().left + messageText.getLocalBounds().width / 2.f, messageText.getLocalBounds().top + messageText.getLocalBounds().height / 2.f);
     messageText.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
}

void Game::showStory(int level) {
    std::string story = "";
     switch (level) {
         case 1: story = "Level 1: Clear the initial asteroid field..."; break;
         case BOSS_LEVEL_INTERVAL: story = "WARNING: Large unidentified object detected!"; break;
         case 4: story = "Level 4: Increased hazard meteor activity reported."; break;
          case BOSS_LEVEL_INTERVAL*2: story = "It's back! And it looks stronger!"; break; // Example for second boss
         // Add more cases
         default:
             // No story for this level, proceed directly
              loadLevel(currentLevel); // Load level immediately
              setState(State::Playing);
             return; // Exit function early
     }
     messageText.setString(story);
     currentState = State::Story; // Set state to Story to display the text
     // Origin/position set in setState(State::Story)
}

// Static collision check method (same as before)
bool Game::isCollide(const Entity *a, const Entity *b) {
    sf::Vector2f diff = b->pos - a->pos; float distSq = diff.x * diff.x + diff.y * diff.y;
    float radiusSum = a->R + b->R; return distSq < (radiusSum * radiusSum);
}