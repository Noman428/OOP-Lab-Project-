#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
#include <sstream>
#include <iostream>
#include <vector>

using namespace sf;

// Base class for all game objects
class GameObject {
protected:
    Vector2f position;

public:
    GameObject(float x, float y) : position(x, y) {}

    virtual void update() = 0; // Pure virtual function for polymorphism
    virtual void draw(RenderWindow& window, Sprite& sprite) = 0;

    Vector2f getPosition() const {
        return position;
    }

    void setPosition(float x, float y) {
        position = {x, y};
    }
};

class Platform : public GameObject {
public:
    Platform(float x, float y) : GameObject(x, y) {}

    void update() override {
        // Platforms only move vertically when the player jumps above a threshold
    }

    void move(float dy) {
        position.y -= dy; // Move platform based on player gravity
    }

    void draw(RenderWindow& window, Sprite& sprite) override {
        sprite.setPosition(position);
        window.draw(sprite);
    }
};

class Player : public GameObject {
private:
    float dx, dy;

public:
    Player() : GameObject(200, 200), dx(0.0f), dy(0.0f) {}

    void moveLeft() {
        position.x -= 5.0f;
    }

    void moveRight() {
        position.x += 5.0f;
    }

    void applyGravity() {
        dy += 0.2f; // Increase gravity
        position.y += dy;
    }

    void jump() {
        dy = -8.0f; // Boost player upwards when colliding with a platform
    }

    void reset() {
        position = {200, 200}; // Reset player position
        dx = 0;
        dy = 0;
    }

    float getVelocityY() const {
        return dy;
    }

    void update() override {
        applyGravity();
    }

    void draw(RenderWindow& window, Sprite& sprite) override {
        sprite.setPosition(position);
        window.draw(sprite);
    }
};

class Game {
private:
    RenderWindow window;
    Texture t1, t2, t3;
    Sprite sBackground, sPlat, sPers;
    Font font;
    Text scoreText, gameOverText, retryText;

    Player player;
    std::vector<Platform> platforms;
    int score;
    bool gameOver;

    // Sound-related variables
    SoundBuffer jumpBuffer;
    Sound jumpSound;

    void resetGame() {
        player.reset();
        score = 0;
        gameOver = false;

        platforms.clear();
        for (int i = 0; i < 10; i++) {
            platforms.emplace_back(rand() % 500, rand() % 700); 
        }   // Adjusted spawn range for platforms

        updateScoreText();
    }

    void updateScoreText() {
        std::ostringstream ss;
        ss << "Score: " << score;
        scoreText.setString(ss.str());
    }

    void handleInput() {
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            player.moveLeft();
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            player.moveRight();
        }
    }

    void handleCollisions() {
        for (auto& platform : platforms) {
            Vector2f pPos = platform.getPosition();
            Vector2f plPos = player.getPosition();

            if ((plPos.x + 50 > pPos.x) && (plPos.x + 20 < pPos.x + 68) &&
                (plPos.y + 70 > pPos.y) && (plPos.y + 70 < pPos.y + 14) &&
                (player.getVelocityY() > 0)) {
                player.jump(); // Boost the player up when they collide with the platform
                jumpSound.play();      // Play the jump sound
            }
        }
    }

    void updatePlatforms() {
        for (auto& platform : platforms) {
            platform.move(player.getVelocityY());

            // Check if platform is off-screen and reset it
            if (platform.getPosition().y > 700) { // Adjusted for new window height
                platform.setPosition(rand() % 500, 0); // Adjusted for new window width
                score++; // Increment score when a platform resets
            }
        }

        updateScoreText(); // Update score text after platform movement
    }

    void draw() {
        window.clear();

        if (gameOver) {
            window.draw(sBackground);
            window.draw(gameOverText);
            window.draw(retryText);
        } else {
            window.draw(sBackground);
            player.draw(window, sPers);
            for (auto& platform : platforms) {
                platform.draw(window, sPlat);
            }
            window.draw(scoreText);
        }

        window.display();
    }

public:				// Adjusted window size
    Game() : window(VideoMode(500, 700), "Doodle Game!"), score(0), gameOver(false) { 
        srand(time(0));
        window.setFramerateLimit(60);

        // Load textures and font
        t1.loadFromFile("images/sea.png");
        t2.loadFromFile("images/platform.png");
        t3.loadFromFile("images/character.png");

        sBackground.setTexture(t1);
        sPlat.setTexture(t2);
        sPers.setTexture(t3);

        if (!font.loadFromFile("fonts/DoodleJumpBold_v2.ttf")) {
            std::cerr << "Failed to load font" << std::endl;
            return;
        }

        // Load sound
        if (!jumpBuffer.loadFromFile("sounds/sound_jump.wav")) {
            std::cerr << "Failed to load sound_jump.wav" << std::endl;
            return;
        }
        jumpSound.setBuffer(jumpBuffer);

        scoreText.setFont(font);
        scoreText.setCharacterSize(30);
        scoreText.setFillColor(Color::Black);
        scoreText.setPosition(10, 10);

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setString("Game Over!");
        gameOverText.setPosition(150, 300);

        retryText.setFont(font);
        retryText.setCharacterSize(28);
        retryText.setFillColor(Color::Black);
        retryText.setString("Press R to Retry or Esc to Exit");
        retryText.setPosition(120, 400);

        resetGame();
    }

    void run() {
        while (window.isOpen()) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed) {
                    window.close();
                }

                if (gameOver) {
                    if (Keyboard::isKeyPressed(Keyboard::R)) {
                        resetGame();
                    } else if (Keyboard::isKeyPressed(Keyboard::Escape)) {
                        window.close();
                    }
                }
            }

            if (!gameOver) {
                handleInput();
                player.update();

                if (player.getPosition().y > 700) { // Adjusted for new window height
                    gameOver = true;
                }

                if (player.getPosition().y < 300) { // Adjusted gravity offset for larger window
                    float offset = 300 - player.getPosition().y;
                    player.setPosition(player.getPosition().x, 300);
                    for (auto& platform : platforms) {
                        platform.move(-offset);
                    }
                }

                updatePlatforms();
                handleCollisions();
            }

            draw();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

