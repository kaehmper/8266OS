#ifndef FLAPPY_GAME_H
#define FLAPPY_GAME_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

enum GameState {
    GAME_GET_READY,
    GAME_PLAYING,
    GAME_OVER
};

class FlappyGame {
public:
    FlappyGame();
    void reset();
    void update(bool clicked, bool longPressed);
    void draw(Adafruit_SSD1306& display);

    bool shouldExit() const { return _shouldExit; }
    void clearExitFlag() { _shouldExit = false; }

private:
    GameState _state;
    bool _shouldExit;

    // Bird physics
    float _birdY;
    float _birdVelocity;
    const float GRAVITY = 0.28f;
    const float JUMP_IMPULSE = -2.4f;
    const uint8_t BIRD_RADIUS = 3;

    // Single active pipe structure
    float _pipeX;
    float _pipeGapY;
    const uint8_t PIPE_WIDTH = 12;
    const uint8_t PIPE_GAP_HEIGHT = 22;
    const float PIPE_SPEED = 1.2f;

    uint32_t _score;
    uint32_t _highScore;
    bool _newHighScore;

    uint32_t _lastFrameTime;

    void spawnPipe();
    bool checkCollision();
};

#endif // FLAPPY_GAME_H
