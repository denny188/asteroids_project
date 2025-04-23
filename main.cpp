#include <SFML/Graphics.hpp>
#include <time.h>      // For srand, rand
#include <list>        // For std::list
#include <string>      // For std::string
#include <vector>      // For std::vector
#include <cmath>       // For cos, sin, sqrt
#include <cstdlib>     // For rand, srand (better C++ practice than time.h alone)
#include <memory>      // (Optional but good practice for future: consider std::unique_ptr)

// Use sf namespace (acceptable in main.cpp, avoid in headers)
using namespace sf;

const int W = 1200;
const int H = 800;

const float DEGTORAD = 0.017453f; // Use const for constants

class Animation
{
public:
    float Frame, speed;
    Sprite sprite;
    std::vector<IntRect> frames;

    // Default constructor
    Animation() : Frame(0.f), speed(0.f) {} // Initialize members

    // Parameterized constructor
    Animation(Texture &t, int x, int y, int w, int h, int count, float Speed)
    {
        Frame = 0;
        speed = Speed;

        frames.reserve(count); // Optimize vector allocation
        for (int i = 0; i < count; i++)
        {
            // Use emplace_back for potentially better performance
            frames.emplace_back(x + i * w, y, w, h);
        }

        sprite.setTexture(t);
        sprite.setOrigin(static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f); // Use float division
        if (!frames.empty()) { // Check if frames vector is not empty
             sprite.setTextureRect(frames[0]);
        }
    }

    void update()
    {
        Frame += speed;
        int n = static_cast<int>(frames.size()); // Explicit cast
        if (Frame >= n) Frame -= n;
        if (n > 0) sprite.setTextureRect(frames[static_cast<int>(Frame)]); // Explicit cast
    }

    bool isEnd() const // Mark as const as it doesn't modify the object
    {
        // Ensure comparison is safe even if frames is empty
        return !frames.empty() && (Frame + speed >= frames.size());
    }
};

class Entity
{
public:
    // Use Vector2f for position for better compatibility with SFML functions
    Vector2f pos;
    Vector2f velocity; // Use velocity instead of dx, dy
    float R;
    float angle;
    bool life;
    std::string name;
    Animation anim;

    Entity() : R(1.f), angle(0.f), life(true) // Initialize members
    {
        // pos and velocity default initialize to (0,0)
    }

    // Use Vector2f for position setting
    void settings(Animation &a, Vector2f startPos, float startAngle = 0.f, float radius = 1.f)
    {
        anim = a;
        pos = startPos;
        angle = startAngle;
        R = radius;
    }

    // Virtual update method
    virtual void update() {};

    // Draw method takes RenderTarget (more generic than RenderWindow)
    void draw(RenderTarget &target)
    {
        anim.sprite.setPosition(pos);
        anim.sprite.setRotation(angle + 90.f); // Ensure float literal
        target.draw(anim.sprite);

        // --- Debug Circle Drawing (Optional) ---
        /*
        CircleShape circle(R);
        circle.setFillColor(Color(255, 0, 0, 170));
        circle.setPosition(pos);
        circle.setOrigin(R, R);
        target.draw(circle);
        */
    }

    // Virtual destructor is important for base classes with virtual functions
    virtual ~Entity() = default; // Use default destructor
};

// --- Derived Entity Classes ---

class Asteroid : public Entity // Use Asteroid instead of asteroid (convention)
{
public:
    Asteroid()
    {
        // Initialize velocity directly
        velocity.x = static_cast<float>(rand() % 8 - 4); // Explicit cast
        velocity.y = static_cast<float>(rand() % 8 - 4); // Explicit cast
        name = "asteroid";
    }

    void update() override // Use override keyword
    {
        pos += velocity;

        // Wrap around screen edges
        if (pos.x > W) pos.x = 0; else if (pos.x < 0) pos.x = W;
        if (pos.y > H) pos.y = 0; else if (pos.y < 0) pos.y = H;
    }
};

class Bullet : public Entity // Use Bullet instead of bullet
{
public:
    Bullet()
    {
        name = "bullet";
    }

    void update() override // Use override keyword
    {
        // Calculate velocity based on angle (ensure using float literals)
        velocity.x = std::cos(angle * DEGTORAD) * 6.f;
        velocity.y = std::sin(angle * DEGTORAD) * 6.f;

        pos += velocity;

        // Check bounds for despawning
        if (pos.x > W || pos.x < 0 || pos.y > H || pos.y < 0)
        {
            life = false;
        }
    }
};

class Player : public Entity // Use Player instead of player
{
public:
    bool thrust;

    Player() : thrust(false) // Initialize thrust
    {
        name = "player";
    }

    void update() override // Use override keyword
    {
        if (thrust)
        {
            velocity.x += std::cos(angle * DEGTORAD) * 0.2f;
            velocity.y += std::sin(angle * DEGTORAD) * 0.2f;
        }
        else
        {
            // Apply damping
            velocity *= 0.99f;
        }

        // Clamp speed
        const float maxSpeed = 15.f;
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > maxSpeed)
        {
            velocity = velocity / speed * maxSpeed; // Normalize and scale
        }

        pos += velocity;

        // Wrap around screen edges
        if (pos.x > W) pos.x = 0; else if (pos.x < 0) pos.x = W;
        if (pos.y > H) pos.y = 0; else if (pos.y < 0) pos.y = H;
    }
};

// --- Collision Function ---

// Takes const pointers as it shouldn't modify entities
bool isCollide(const Entity *a, const Entity *b)
{
    // Use Vector2f's length calculation if available or stick to manual
    Vector2f diff = b->pos - a->pos;
    float distSq = diff.x * diff.x + diff.y * diff.y; // Squared distance
    float radiusSum = a->R + b->R;
    return distSq < (radiusSum * radiusSum);
}

// --- Main Function ---

int main()
{
    // Seed random number generator once
    srand(static_cast<unsigned int>(time(0))); // Explicit cast

    // Create the main window
    RenderWindow app(VideoMode(W, H), "Asteroids! (SFML 3.0 Port)");
    app.setFramerateLimit(60);

    // Load textures
    Texture t1, t2, t3, t4, t5, t6, t7;
    if (!t1.loadFromFile("images/spaceship.png")) return EXIT_FAILURE;
    if (!t2.loadFromFile("images/background.jpg")) return EXIT_FAILURE;
    if (!t3.loadFromFile("images/explosions/type_C.png")) return EXIT_FAILURE;
    if (!t4.loadFromFile("images/rock.png")) return EXIT_FAILURE;
    if (!t5.loadFromFile("images/fire_blue.png")) return EXIT_FAILURE;
    if (!t6.loadFromFile("images/rock_small.png")) return EXIT_FAILURE;
    if (!t7.loadFromFile("images/explosions/type_B.png")) return EXIT_FAILURE;

    // Removed tN.setSmooth(true); as it's likely deprecated/removed in SFML 3.0

    // Create background sprite
    Sprite background(t2);

    // Create animations
    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5f);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2f);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2f);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8f);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0.f);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0.f);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5f);

    // Use std::list of unique_ptr for automatic memory management (Recommended)
    std::list<std::unique_ptr<Entity>> entities;

    // Create initial asteroids
    for (int i = 0; i < 15; i++)
    {
        auto asteroid = std::make_unique<Asteroid>(); // Create unique_ptr
        Vector2f pos(static_cast<float>(rand() % W), static_cast<float>(rand() % H));
        float angle = static_cast<float>(rand() % 360);
        asteroid->settings(sRock, pos, angle, 25.f);
        entities.push_back(std::move(asteroid)); // Move pointer into list
    }

    // Create player - store raw pointer temporarily for easy access
    Player* p = new Player(); // Create raw pointer first
    p->settings(sPlayer, Vector2f(200.f, 200.f), 0.f, 20.f);
    entities.push_back(std::unique_ptr<Player>(p)); // Transfer ownership to unique_ptr in list

    // ----- Main Game Loop -----
    while (app.isOpen())
    {
        // --- Event Handling ---
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Space)
                {
                    auto bullet = std::make_unique<Bullet>();
                    // Use player's current position and angle
                    bullet->settings(sBullet, p->pos, p->angle, 10.f);
                    entities.push_back(std::move(bullet));
                }
            }
        }

        // --- Player Input ---
        if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += 3.f;
        if (Keyboard::isKeyPressed(Keyboard::Left)) p->angle -= 3.f;
        p->thrust = Keyboard::isKeyPressed(Keyboard::Up);

        // --- Collision Detection ---
        // Use iterators for safe removal during iteration
        for (auto i = entities.begin(); i != entities.end(); ++i)
        {
            for (auto j = std::next(i); j != entities.end(); ++j) // Avoid self-collision and duplicate checks
            {
                Entity* ent1 = i->get(); // Get raw pointer from unique_ptr
                Entity* ent2 = j->get(); // Get raw pointer from unique_ptr

                if (isCollide(ent1, ent2))
                {
                    // Asteroid <-> Bullet Collision
                    if ((ent1->name == "asteroid" && ent2->name == "bullet") ||
                        (ent1->name == "bullet" && ent2->name == "asteroid"))
                    {
                        ent1->life = false;
                        ent2->life = false;

                        // Create explosion at asteroid position
                        auto explosion = std::make_unique<Entity>();
                        // Determine which one was the asteroid for position/radius
                        Entity* asteroidPtr = (ent1->name == "asteroid") ? ent1 : ent2;
                        explosion->settings(sExplosion, asteroidPtr->pos);
                        explosion->name = "explosion";
                        // Add explosion later to avoid iterator issues immediately

                        // Create smaller asteroids if it wasn't already small
                        if (asteroidPtr->R > 15.f) { // Check radius
                             for(int k=0; k<2; k++) {
                                 auto smallAsteroid = std::make_unique<Asteroid>();
                                 smallAsteroid->settings(sRock_small, asteroidPtr->pos, static_cast<float>(rand()%360), 15.f);
                                 // Add small asteroids later
                             }
                        }
                    }
                    // Player <-> Asteroid Collision
                    else if ((ent1->name == "player" && ent2->name == "asteroid") ||
                             (ent1->name == "asteroid" && ent2->name == "player"))
                    {
                        // Mark asteroid for removal
                        Entity* asteroidPtr = (ent1->name == "asteroid") ? ent1 : ent2;
                        asteroidPtr->life = false;

                        // Create ship explosion at player position
                        auto explosion = std::make_unique<Entity>();
                        explosion->settings(sExplosion_ship, p->pos);
                        explosion->name = "explosion";
                        // Add explosion later

                        // Reset player
                        p->settings(sPlayer, Vector2f(W / 2.f, H / 2.f), 0.f, 20.f);
                        p->velocity = Vector2f(0.f, 0.f); // Reset velocity
                    }
                }
            }
        }

        // --- Update Player Animation ---
        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;

        // --- Update Entities and Remove Dead Ones ---
        // Need a separate list for entities created during collision pass
        std::list<std::unique_ptr<Entity>> newEntities;

        for (auto i = entities.begin(); i != entities.end(); /* no increment here */)
        {
            Entity* e = i->get();

            // Handle collisions resulting in new entities (from previous loop)
             if (e->name == "explosion" && e->anim.isEnd()) {
                 e->life = false; // Mark explosion for removal when animation finishes
             }
            // (Logic for adding new explosions/asteroids needs refinement - moved adding to separate loop)


            e->update();    // Update entity logic (position, etc.)
            e->anim.update(); // Update entity animation frame

            if (!e->life)
            {
                // If asteroid died, potentially create explosion/small asteroids NOW
                 if (e->name == "asteroid") {
                     // Create explosion
                     auto explosion = std::make_unique<Entity>();
                     explosion->settings(sExplosion, e->pos);
                     explosion->name = "explosion";
                     newEntities.push_back(std::move(explosion));

                     // Create smaller asteroids if needed
                      if (e->R > 15.f) {
                          for(int k=0; k<2; k++) {
                              auto smallAsteroid = std::make_unique<Asteroid>();
                              smallAsteroid->settings(sRock_small, e->pos, static_cast<float>(rand()%360), 15.f);
                              newEntities.push_back(std::move(smallAsteroid));
                          }
                      }
                 } else if (e->name == "player") { // Player died in collision
                      // Create ship explosion
                      auto explosion = std::make_unique<Entity>();
                      explosion->settings(sExplosion_ship, e->pos); // Use player's last pos
                      explosion->name = "explosion";
                      newEntities.push_back(std::move(explosion));
                       // Player reset is handled during collision check
                 }


                i = entities.erase(i); // Erase returns iterator to the next element
                // No need to delete e; unique_ptr handles it
            }
            else
            {
                ++i; // Only increment if not erased
            }
        }

        // Add newly created entities (explosions, small asteroids)
        entities.splice(entities.end(), newEntities);


        // --- Spawn New Asteroids Occasionally ---
        if (rand() % 150 == 0)
        {
            auto asteroid = std::make_unique<Asteroid>();
            Vector2f pos(0.f, static_cast<float>(rand() % H)); // Start off-screen left
            float angle = static_cast<float>(rand() % 360);
            asteroid->settings(sRock, pos, angle, 25.f);
            entities.push_back(std::move(asteroid));
        }

        // --- Drawing ---
        app.clear(Color::Black); // Clear with black color
        app.draw(background);

        // Draw all entities
        for (const auto& entityPtr : entities) // Use const reference
        {
            entityPtr->draw(app); // Draw using the raw pointer from unique_ptr
        }

        app.display(); // Update the window
    }

    return EXIT_SUCCESS; // Indicate successful execution
}