#include "FlappyGame.h"
#include "Config.h"

FlappyGame::FlappyGame() : _state(GAME_GET_READY), _shouldExit(false) {
    reset();
}

void FlappyGame::reset() {
    _state = GAME_GET_READY;
    _shouldExit = false;
    _birdY = 32.0f;
    _birdVelocity = 0.0f;
    _score = 0;
    _newHighScore = false;

    // Load high score from persistent config
    _highScore = Config::getInstance().getSettings().gameHighScore;

    spawnPipe();
    _lastFrameTime = millis();
}

void FlappyGame::spawnPipe() {
    _pipeX = 128.0f;
    // Gap Y should center between 15 and 48
    _pipeGapY = 15 + random(0, 27);
}

bool FlappyGame::checkCollision() {
    // Top and bottom boundaries (screen height is 64)
    if (_birdY - BIRD_RADIUS < 0 || _birdY + BIRD_RADIUS > 63) {
        return true;
    }

    // Pipe collision checking
    // Bird is at X = 20
    float birdLeft = 20.0f - BIRD_RADIUS;
    float birdRight = 20.0f + BIRD_RADIUS;
    float birdTop = _birdY - BIRD_RADIUS;
    float birdBottom = _birdY + BIRD_RADIUS;

    float pipeLeft = _pipeX;
    float pipeRight = _pipeX + PIPE_WIDTH;

    // Check if bird is horizontally within the pipe
    if (birdRight >= pipeLeft && birdLeft <= pipeRight) {
        // Check if bird hits upper pipe or lower pipe
        float upperPipeBottom = _pipeGapY - (PIPE_GAP_HEIGHT / 2);
        float lowerPipeTop = _pipeGapY + (PIPE_GAP_HEIGHT / 2);

        if (birdTop <= upperPipeBottom || birdBottom >= lowerPipeTop) {
            return true;
        }
    }

    return false;
}

void FlappyGame::update(bool clicked, bool longPressed) {
    // Enforce fixed frame rate timing (~30 FPS or about 33ms)
    uint32_t now = millis();
    if (now - _lastFrameTime < 30) {
        return;
    }
    _lastFrameTime = now;

    if (longPressed) {
        _shouldExit = true;
        return;
    }

    switch (_state) {
        case GAME_GET_READY:
            // Gently float bird up and down using a sine wave
            _birdY = 32.0f + sinf((float)now / 150.0f) * 4.0f;
            if (clicked) {
                _state = GAME_PLAYING;
                _birdVelocity = JUMP_IMPULSE;
            }
            break;

        case GAME_PLAYING: {
            if (clicked) {
                _birdVelocity = JUMP_IMPULSE;
            }

            // Apply gravity
            _birdVelocity += GRAVITY;
            _birdY += _birdVelocity;

            // Move pipe
            float prevX = _pipeX;
            _pipeX -= PIPE_SPEED;

            // Check if pipe went off-screen
            if (_pipeX < -PIPE_WIDTH) {
                spawnPipe();
            }

            // Check score
            if (prevX >= 20.0f && _pipeX < 20.0f) {
                _score++;
                if (_score > _highScore) {
                    _highScore = _score;
                    _newHighScore = true;
                }
            }

            // Check collision
            if (checkCollision()) {
                _state = GAME_OVER;

                // Save high score if updated
                if (_newHighScore) {
                    SystemSettings& settings = Config::getInstance().getSettings();
                    settings.gameHighScore = _highScore;
                    Config::getInstance().save();
                }
            }
            break;
        }

        case GAME_OVER:
            if (clicked) {
                reset();
            }
            break;
    }
}

void FlappyGame::draw(Adafruit_SSD1306& display) {
    display.clearDisplay();

    // 1. Draw Background border
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);

    // 2. Draw Bird
    display.fillCircle(20, (int16_t)_birdY, BIRD_RADIUS, SSD1306_WHITE);
    // Add eye
    display.drawPixel(21, (int16_t)_birdY - 1, SSD1306_BLACK);

    // 3. Draw Pipe (only in get-ready and playing states)
    if (_state == GAME_GET_READY || _state == GAME_PLAYING) {
        int16_t upperPipeHeight = (int16_t)(_pipeGapY - (PIPE_GAP_HEIGHT / 2));
        int16_t lowerPipeTop = (int16_t)(_pipeGapY + (PIPE_GAP_HEIGHT / 2));

        // Upper pipe
        display.fillRect((int16_t)_pipeX, 1, PIPE_WIDTH, upperPipeHeight, SSD1306_WHITE);
        display.drawRect((int16_t)_pipeX, 1, PIPE_WIDTH, upperPipeHeight, SSD1306_BLACK);

        // Lower pipe
        display.fillRect((int16_t)_pipeX, lowerPipeTop, PIPE_WIDTH, 63 - lowerPipeTop, SSD1306_WHITE);
        display.drawRect((int16_t)_pipeX, lowerPipeTop, PIPE_WIDTH, 63 - lowerPipeTop, SSD1306_BLACK);
    }

    // 4. Draw Score
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(4, 4);
    display.print("S:");
    display.print(_score);

    display.setCursor(80, 4);
    display.print("Hi:");
    display.print(_highScore);

    // 5. Draw state overlays
    if (_state == GAME_GET_READY) {
        // Centered "GET READY" box
        display.fillRect(25, 20, 78, 24, SSD1306_BLACK);
        display.drawRect(25, 20, 78, 24, SSD1306_WHITE);

        display.setCursor(32, 24);
        display.print("FLAPPY ESP");
        display.setCursor(30, 33);
        display.print("Click to Jump");
    } else if (_state == GAME_OVER) {
        // Centered "GAME OVER" box
        display.fillRect(15, 14, 98, 36, SSD1306_BLACK);
        display.drawRect(15, 14, 98, 36, SSD1306_WHITE);

        display.setCursor(34, 18);
        display.print("GAME OVER");

        if (_newHighScore) {
            display.setCursor(24, 27);
            display.print("NEW HIGH SCORE!");
        } else {
            display.setCursor(26, 27);
            display.print("Score: ");
            display.print(_score);
        }

        display.setCursor(20, 38);
        display.print("Press: New Game");
    }
}
