#include "Player.h"
#include "ResourceManager.h"
#include <cmath>
#include <iostream>

const float PLAYER_DEGTORAD = 0.017453f;

Player::Player() :
    thrust(false),
    maxSpeed(15.f),
    acceleration(0.2f),
    turnSpeed(180.f),
    friction(0.99f),
    shootCooldown(0.25f),
    shootTimer(0.f),
    score(0),
    lives(3),
    currentShipType(ShipType::Standard),
    currentWeaponType(Bullet::BulletType::Standard), // Start with standard bullets
    shieldActive(false),
    shieldTimer(0.f),
    speedBoostTimer(0.f),
    slowTimer(0.f),
    reverseControlsActive(false),
    reverseControlsTimer(0.f),
    weaponPowerUpActive(false),
    weaponPowerUpTimer(0.f),
    shieldTexturePtr(nullptr), // Initialize texture pointers
    weaponEffectTexturePtr(nullptr),
    speedEffectTexturePtr(nullptr)
{
    type = Type::Player;
    name = "player";
}

// Override settings để load đúng 2 trạng thái từ spaceship.png
void Player::settings(Animation &a, sf::Vector2f startPos, float startAngle, float radius) {
    // Không dùng animation 'a' được truyền vào nữa, vì chúng ta tự định nghĩa anim từ spaceship.png
    try {
        sf::Texture& playerTexture = ResourceManager::getInstance().getTexture("spaceship.png");
        int textureWidth = playerTexture.getSize().x; // ~250
        int textureHeight = playerTexture.getSize().y; // ~500
        int frameWidth = textureWidth; // Toàn bộ chiều rộng là 1 frame
        int frameHeight = textureHeight / 2; // Chia đôi chiều cao cho 2 trạng thái (~250)

        std::cout << "Setting up player animations from spaceship.png ("
                  << textureWidth << "x" << textureHeight << ", Frame H: " << frameHeight << ")" << std::endl;

        // Khởi tạo anim_idle (phần trên của texture)
        // Animation(texture, x, y, w, h, count, speed, loop)
        anim_idle = Animation(playerTexture, 0, 0, frameWidth, frameHeight, 1, 0.f, false);

        // Khởi tạo anim_thrust (phần dưới của texture)
        anim_thrust = Animation(playerTexture, 0, frameHeight, frameWidth, frameHeight, 1, 0.f, false);

        // Đặt animation ban đầu là idle
        this->anim = anim_idle; // Quan trọng: gán anim hiện tại cho Entity base class

        // *** THÊM SCALING Ở ĐÂY ***
        float targetVisualHeight = 60.0f; // Đặt chiều cao mong muốn (ví dụ: 60 pixels)
        float scaleFactor = targetVisualHeight / static_cast<float>(frameHeight);
        std::cout << "Player Scale Factor: " << scaleFactor << std::endl;

        // Scale cả hai sprite animation
        anim_idle.sprite.setScale(scaleFactor, scaleFactor);
        anim_thrust.sprite.setScale(scaleFactor, scaleFactor);
        // Gán lại anim hiện tại để đảm bảo sprite trong Entity base cũng được scale
        this->anim = anim_idle;

        // Load textures cho hiệu ứng power-up (giữ nguyên logic này)
        shieldTexturePtr = &ResourceManager::getInstance().getTexture("shield_powerup.png");
        weaponEffectTexturePtr = &ResourceManager::getInstance().getTexture("weapon_powerup.png");
        speedEffectTexturePtr = &ResourceManager::getInstance().getTexture("speed_powerup.png");
        // Setup effect sprites
        if (shieldTexturePtr) {
            shieldEffectSprite.setTexture(*shieldTexturePtr);
            // Origin của shield effect nên dựa trên bán kính R + offset mong muốn,
            // thay vì kích thước texture gốc (vì texture shield gốc rất lớn)
            // Đặt origin tương đối ở giữa bán kính hiển thị mong muốn
             float shieldVisualRadius = this->R + 5.f; // Bán kính trực quan của shield
             shieldEffectSprite.setOrigin(shieldVisualRadius, shieldVisualRadius); // Origin dựa trên bán kính
             // Scale sẽ được đặt trong Player::draw dựa trên R và pulsing effect
             shieldEffectSprite.setColor(sf::Color(255, 255, 255, 100)); // Semi-transparent
        }
        if (weaponEffectTexturePtr) {
            weaponEffectSprite.setTexture(*weaponEffectTexturePtr);
            weaponEffectSprite.setOrigin(weaponEffectTexturePtr->getSize().x / 2.f, weaponEffectTexturePtr->getSize().y / 2.f);
            weaponEffectSprite.setScale(0.3f, 0.3f); // Scale nhỏ lại để làm overlay
            weaponEffectSprite.setColor(sf::Color(255, 255, 255, 150));
        }
        if (speedEffectTexturePtr) {
            speedEffectSprite.setTexture(*speedEffectTexturePtr);
            // Đặt origin ở giữa bên phải để gắn vào đuôi tàu? Hoặc giữ ở giữa
            speedEffectSprite.setOrigin(speedEffectTexturePtr->getSize().x / 2.f, speedEffectTexturePtr->getSize().y / 2.f);
             speedEffectSprite.setScale(0.5f, 0.5f); // Scale nhỏ hiệu ứng đuôi
             speedEffectSprite.setColor(sf::Color(255, 255, 255, 180));
        }


    } catch (const std::runtime_error& e) {
        std::cerr << "Error setting player animations/effects: " << e.what() << std::endl;
        // Fallback nếu có lỗi (dùng 'a' nếu cần, nhưng lý tưởng là báo lỗi và thoát)
        this->anim = a; // Dùng animation mặc định nếu lỗi
    }

    // Gọi base class settings với anim *đã được gán đúng* (anim_idle ban đầu)
    // và các thông số khác
    Entity::settings(this->anim, startPos, startAngle, radius);
    // Reset lại các trạng thái khác của Player (như cũ)
    reset();
    score = 0;
    lives = 3;
}

void Player::update(float dt, const sf::Vector2u& windowSize) {
    if (!life) return;

    handleInput(dt);
    updateEffects(dt);
    applyMovement(dt, windowSize);

    // *** Logic chuyển đổi Animation cốt lõi ***
    // Chỉ cần gán đúng đối tượng Animation (anim_idle hoặc anim_thrust)
    // cho biến 'anim' của lớp Entity base.
    anim = thrust ? anim_thrust : anim_idle;

    // Không cần gọi anim.update() vì speed = 0 và count = 1, frame sẽ không thay đổi.
    // Tuy nhiên, gọi cũng không sao.
    // anim.update(dt); // Có thể bỏ dòng này

    if (shootTimer > 0) {
        shootTimer -= dt;
    }
}

void Player::handleInput(float dt) {
    float actualTurnSpeed = turnSpeed * (slowTimer > 0 ? 0.5f : 1.0f); // Slow effect on turning
    bool leftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

    if (reverseControlsActive) {
        std::swap(leftPressed, rightPressed); // Swap input effect
    }

    if (leftPressed) {
        angle -= actualTurnSpeed * dt;
    }
    if (rightPressed) {
        angle += actualTurnSpeed * dt;
    }

    thrust = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);

    // Shooting input check (actual spawning happens in Game::spawnBullet called from Game::handleInput)
    // Just checking readiness here
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer <= 0) {
    //     shoot(); // Signal intent to shoot
    // }
}

// Signal that the player wants to shoot (Game class will handle cooldown and spawning)
void Player::shoot() {
     //if (shootTimer <= 0) {
         //shootTimer = shootCooldown; // Reset cooldown internally
         // Game class will now call spawnBullet
     //}
     std::cout << "Player::shoot() called (intent signal)." << std::endl; // DEBUG
}


void Player::applyMovement(float dt, const sf::Vector2u& windowSize) {
     float currentMaxSpeed = maxSpeed * (speedBoostTimer > 0 ? 1.5f : 1.0f) * (slowTimer > 0 ? 0.5f : 1.0f);
     float currentAcceleration = acceleration * (slowTimer > 0 ? 0.5f : 1.0f);

    if (thrust) {
        velocity.x += std::cos((angle - 90) * PLAYER_DEGTORAD) * currentAcceleration * 60.f * dt;
        velocity.y += std::sin((angle - 90) * PLAYER_DEGTORAD) * currentAcceleration * 60.f * dt;
    } else {
         velocity *= std::pow(friction, dt * 60.f);
    }

    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (speed > currentMaxSpeed) {
        velocity = (velocity / speed) * currentMaxSpeed;
    }

    pos += velocity * dt * 60.f;

    // Screen wrapping
    if (pos.x > windowSize.x + R) pos.x = -R; else if (pos.x < -R) pos.x = windowSize.x + R;
    if (pos.y > windowSize.y + R) pos.y = -R; else if (pos.y < -R) pos.y = windowSize.y + R;
}

void Player::updateEffects(float dt) {
     if (shieldActive) {
        shieldTimer -= dt;
        if (shieldTimer <= 0) shieldActive = false;
     }
     if (speedBoostTimer > 0) {
         speedBoostTimer -= dt;
     }
      if (slowTimer > 0) {
         slowTimer -= dt;
     }
     if (reverseControlsActive) {
         reverseControlsTimer -= dt;
         if (reverseControlsTimer <= 0) reverseControlsActive = false;
     }
      if (weaponPowerUpActive) {
         weaponPowerUpTimer -= dt;
         if (weaponPowerUpTimer <= 0) weaponPowerUpActive = false;
     }
}


void Player::reset() {
    // Don't reset position/angle here, Game::loadLevel or respawn logic handles it
    velocity = sf::Vector2f(0.f, 0.f);
    life = true; // Should be alive after reset
    shieldActive = false;
    shieldTimer = 0.f;
    speedBoostTimer = 0.f;
    slowTimer = 0.f;
    reverseControlsActive = false;
    reverseControlsTimer = 0.f;
    weaponPowerUpActive = false;
    weaponPowerUpTimer = 0.f;
    currentWeaponType = Bullet::BulletType::Standard; // Reset weapon
    shootCooldown = 0.25f; // Reset shoot speed
    shootTimer = 0.f;
    // Score and lives are reset in Game::resetGame or Game::startSurvival
}

void Player::takeDamage() {
    if (!life) return; // Tránh gọi nhiều lần
    if (shieldActive) { /* ... shield logic ... */ return; }

    lives--;
    std::cout << "Player took damage. Lives remaining: " << lives << std::endl; // DEBUG
    if (lives <= 0) {
        life = false; // Chỉ thực sự "chết" (cần cleanup) khi hết mạng
        std::cout << "Player has no lives left. Setting life = false." << std::endl; // DEBUG
    } else {
        // Vẫn còn mạng, chỉ cần reset vị trí và trạng thái, không set life = false
        // Logic reset vị trí và trạng thái sẽ nằm trong Game::updatePlaying khi timer hết
        life = false; // *** Vẫn cần set life=false để dừng hoạt động tạm thời ***
         std::cout << "Player has lives left, setting life = false temporarily for respawn." << std::endl; // DEBUG
    }
}

void Player::addScore(int points) {
    score += points;
}

void Player::applyPowerUp(PowerUp* item) {
     if (!item) return;

     if (!item->getIsPowerDown()) {
         // Apply Power-Up
         switch (item->getPowerUpType()) {
            case PowerUp::PowerUpType::Shield:
                shieldActive = true;
                shieldTimer = item->duration; // Use duration from item
                break;
            case PowerUp::PowerUpType::Weapon:
                 // TODO: Cycle through weapon types or specific upgrade logic
                 if (currentWeaponType == Bullet::BulletType::Standard) {
                     currentWeaponType = Bullet::BulletType::Laser; // Example upgrade
                     shootCooldown = 0.15f; // Laser might shoot faster
                 } else if (currentWeaponType == Bullet::BulletType::Laser) {
                      // currentWeaponType = Bullet::BulletType::Spread;
                      // shootCooldown = 0.4f; // Spread might be slower
                 }
                 weaponPowerUpActive = true;
                 weaponPowerUpTimer = item->duration;
                 break;
            case PowerUp::PowerUpType::Speed:
                 speedBoostTimer = item->duration;
                 break;
             case PowerUp::PowerUpType::ExtraLife:
                  lives++;
                  break;
         }
     } else {
         // Apply Power-Down
         switch (item->getPowerDownType()) {
             case PowerUp::PowerDownType::Slow:
                 slowTimer = item->duration;
                 // Reset speed boost if active
                 speedBoostTimer = 0.f;
                 break;
             case PowerUp::PowerDownType::ReverseControls:
                 reverseControlsActive = true;
                 reverseControlsTimer = item->duration;
                 break;
             case PowerUp::PowerDownType::WeakerWeapon:
                  // TODO: Implement logic to downgrade weapon or reduce damage
                  currentWeaponType = Bullet::BulletType::Standard;
                  shootCooldown = 0.35f; // Slower shooting
                  break;
         }
     }
}

void Player::setShipType(ShipType type) {
    // Same as before
    currentShipType = type;
    switch (type) {
        case ShipType::Standard: maxSpeed = 15.f; acceleration = 0.2f; turnSpeed = 180.f; break;
        case ShipType::Fast: maxSpeed = 20.f; acceleration = 0.18f; turnSpeed = 220.f; break;
        case ShipType::Heavy: maxSpeed = 12.f; acceleration = 0.25f; turnSpeed = 150.f; break;
    }
}

// Override draw to add effects
void Player::draw(sf::RenderTarget &target) {
    if (!life) return;

    // Draw base ship
    Entity::draw(target); // Calls base draw which draws anim.sprite

    // Draw Shield Effect
    if (shieldActive && shieldTexturePtr) {
        shieldEffectSprite.setPosition(pos);
        // Optional: Add pulsing/rotating effect
        shieldEffectSprite.rotate(1.f); // Slow rotation
        float scaleFactor = 1.0f + 0.05f * std::sin(shieldTimer * 5.f); // Simple pulse
        shieldEffectSprite.setScale(scaleFactor * (R + 5.f) / (shieldTexturePtr->getSize().x / 2.f), // Scale based on player R
                                   scaleFactor * (R + 5.f) / (shieldTexturePtr->getSize().y / 2.f));
        target.draw(shieldEffectSprite);
    }

    // Draw Weapon Power-up Indicator
    if (weaponPowerUpActive && weaponEffectTexturePtr) {
         weaponEffectSprite.setPosition(pos); // Center on player
         weaponEffectSprite.setRotation(angle + 90.f); // Rotate with player
         target.draw(weaponEffectSprite);
    }

     // Draw Speed Boost Effect (at the back) - More complex positioning
     if (speedBoostTimer > 0 && speedEffectTexturePtr && thrust) { // Only show when thrusting with boost
         float backOffset = -R * 0.8f; // Position behind the center
         float angleRad = (angle - 90) * PLAYER_DEGTORAD;
         sf::Vector2f offsetVec(std::cos(angleRad) * backOffset, std::sin(angleRad) * backOffset);
         speedEffectSprite.setPosition(pos + offsetVec);
         speedEffectSprite.setRotation(angle + 90.f); // Align with ship
         // Optional: Animate sprite or color based on timer
         target.draw(speedEffectSprite);
     }
}