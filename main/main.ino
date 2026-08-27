#include "BoltOS.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

CPU cpu;
Kernel kernel(cpu);

const int PIN_X = A0;
const int PIN_Y = A1;
const int PIN_SW = 2;
const int PIN_GAME_BTN = 4;

bool lastGameBtnState = HIGH;
bool isGameRunning = false;
const uint8_t boxSize = 12;

typedef bool (*GameFunction)();

struct Game {
  char icon;
  GameFunction launch;
  uint8_t x;
  uint8_t y;
};

bool spaceShooter();
bool clickerGame();
bool snakeGame();
bool pingPongGame();
bool flappyGame();
bool mazeGame();
bool dodgeGame();
bool dinoGame();

Game myGames[] = {
  { (char)2, spaceShooter },
  { (char)15, clickerGame },
  { (char)3, snakeGame },
  { (char)4, pingPongGame },
  { (char)5, flappyGame },
  { (char)6, mazeGame },
  { (char)7, dodgeGame },
  { (char)8, dinoGame }
};

const uint8_t TOTAL_GAMES = sizeof(myGames) / sizeof(Game);

void drawBox(uint8_t x, uint8_t y, uint8_t s, char name);
bool distance(byte mouseX, byte mouseY, uint8_t boxX, uint8_t boxY);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for (;;)
      ;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_GAME_BTN, INPUT_PULLUP);

  for (uint8_t i = 0; i < TOTAL_GAMES; i++) {
    myGames[i].x = 4 + (i % 4) * 31;
    myGames[i].y = 4 + (i / 4) * 16;
  }

  randomSeed(analogRead(A2));
}

void loop() {
  display.clearDisplay();

  int rawX = analogRead(PIN_X);
  int rawY = analogRead(PIN_Y);

  byte x = map(rawX, 0, 1023, 0, 122);
  byte y = map(rawY, 0, 1023, 0, 56);

  if (!isGameRunning) {
    for (uint8_t i = 0; i < TOTAL_GAMES; i++) {
      drawBox(myGames[i].x, myGames[i].y, boxSize, myGames[i].icon);
    }

    display.drawChar(x, y, (char)24, SSD1306_WHITE, SSD1306_BLACK, 1);
  }

  bool currentGameBtnState = digitalRead(PIN_GAME_BTN);
  if (currentGameBtnState == LOW && lastGameBtnState == HIGH) {
    if (!isGameRunning) {
      for (uint8_t i = 0; i < TOTAL_GAMES; i++) {
        if (distance(x, y, myGames[i].x, myGames[i].y)) {
          isGameRunning = true;
          delay(200);

          kernel.addTask(new Task(5, TaskState::READY, myGames[i].launch));
          break;
        }
      }
    }
  }
  lastGameBtnState = currentGameBtnState;

  kernel.run();

  display.display();
  delay(30);
}

bool distance(byte mouseX, byte mouseY, uint8_t boxX, uint8_t boxY) {
  return (mouseX >= boxX && mouseX <= (boxX + boxSize) && mouseY >= boxY && mouseY <= (boxY + boxSize));
}

void drawBox(uint8_t x, uint8_t y, uint8_t s, char name) {
  display.drawRect(x, y, s, s, SSD1306_WHITE);
  uint8_t fontWidth{ 5 }, fontHeight{ 7 }, fontSize{ 1 };
  int16_t charX{ x + (s - (fontWidth * fontSize)) / 2 };
  int16_t charY{ y + (s - (fontHeight * fontSize)) / 2 };
  display.drawChar(charX, charY, name, SSD1306_WHITE, SSD1306_BLACK, fontSize);
}

struct bullet {
  int x, y;
  bool active;
  void go() {
    y -= 2;
  }
};
struct enemy {
  int x, y;
  bool active;
  void go() {
    y += 1;
  }
};

bool spaceShooter() {
  static int score = 0;
  static unsigned long lastEnemySpawnTime = 0;
  static bool lastButtonState = HIGH;
  static bool initialized = false;

  const byte MAX_BULLETS = 10;
  static bullet bullets[MAX_BULLETS];
  const byte MAX_ENEMIES = 6;
  static enemy bots[MAX_ENEMIES];

  if (!initialized) {
    score = 0;
    lastEnemySpawnTime = 0;
    for (byte i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (byte i = 0; i < MAX_ENEMIES; i++) bots[i].active = false;
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  int rawX = analogRead(PIN_X);
  int buttonState = digitalRead(PIN_SW);
  int xCoord = map(rawX, 0, 1023, 0, 120);
  byte yCoord = 50;

  display.clearDisplay();

  if (millis() - lastEnemySpawnTime > 1500) {
    for (byte i = 0; i < MAX_ENEMIES; i++) {
      if (!bots[i].active) {
        bots[i].x = random(0, SCREEN_WIDTH - 8);
        bots[i].y = 0;
        bots[i].active = true;
        lastEnemySpawnTime = millis();
        break;
      }
    }
  }

  if (buttonState == LOW && lastButtonState == HIGH) {
    for (byte i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = xCoord + 2;
        bullets[i].y = yCoord - 4;
        bullets[i].active = true;
        break;
      }
    }
  }
  lastButtonState = buttonState;

  for (byte i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].go();
      if (bullets[i].y < 0) bullets[i].active = false;
      else {
        display.setCursor(bullets[i].x, bullets[i].y);
        display.print((char)9);
      }
    }
  }

  for (byte i = 0; i < MAX_ENEMIES; i++) {
    if (bots[i].active) {
      bots[i].go();
      if (bots[i].y > SCREEN_HEIGHT) {
        bots[i].active = false;
        score = 0;
      } else {

        for (byte j = 0; j < MAX_BULLETS; j++) {
          if (bullets[j].active) {
            if (abs(bullets[j].x - bots[i].x) < 6 && abs(bullets[j].y - bots[i].y) < 6) {
              bots[i].active = false;
              bullets[j].active = false;
              score += 1;
            }
          }
        }

        if (abs(xCoord - bots[i].x) < 6 && abs(yCoord - bots[i].y) < 6) {
          bots[i].active = false;
          score = 0;
        }
        if (bots[i].active) {
          display.setCursor(bots[i].x, bots[i].y);
          display.print((char)2);
        }
      }
    }
  }

  display.setCursor(xCoord, yCoord);
  display.print((char)30);
  display.setCursor(0, 0);
  display.print("Score: ");
  display.print(score);

  return false;
}

bool clickerGame() {
  static long long clicks = 0;
  static long long clickPower = 1;
  static long long autoClickers = 0;

  static long long upgrade1Cost = 15;
  static long long upgrade2Cost = 100;
  static long long upgrade3Cost = 500;

  static long long clickMultiplier = 1;

  static bool lastSwState = HIGH;
  static bool lastGameBtnState = HIGH;
  static bool initialized = false;
  static unsigned long lastAutoClickTime = 0;

  static int menuSelection = 0;
  static int shopIndex = 0;

  if (!initialized) {
    clicks = 0;
    clickPower = 1;
    autoClickers = 0;
    clickMultiplier = 1;
    upgrade1Cost = 15;
    upgrade2Cost = 100;
    upgrade3Cost = 500;
    lastAutoClickTime = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW && lastGameBtnState == HIGH) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }
  lastGameBtnState = digitalRead(PIN_GAME_BTN);

  if (millis() - lastAutoClickTime >= 1000) {
    lastAutoClickTime = millis();
    clicks += autoClickers * clickMultiplier;
  }

  int rawX = analogRead(PIN_X);
  int rawY = analogRead(PIN_Y);
  bool currentSwState = digitalRead(PIN_SW);

  if (rawX < 200) {
    menuSelection = 0;
    delay(150);
  } else if (rawX > 800) {
    menuSelection = 1;
    delay(150);
  }

  display.clearDisplay();

  if (menuSelection == 0) {
    if (currentSwState == LOW && lastSwState == HIGH) {
      clicks += clickPower * clickMultiplier;
    }

    display.setCursor(5, 2);
    display.print("CLICKS: ");
    display.print(clicks);

    display.setCursor(5, 16);
    display.print("Power: +");
    display.print(clickPower);
    
    display.setCursor(5, 26);
    display.print("Auto: ");
    display.print(autoClickers);
    display.print("/s");

    display.setCursor(5, 36);
    display.print("Multi: x");
    display.print(clickMultiplier);

    display.setCursor(5, 48);
    display.print("[Click SW to Mine]");
    display.setCursor(5, 55);
    display.print("[Joy L/R: Shop]");

  } else {
    if (rawY < 200) {
      shopIndex = (shopIndex - 1 + 3) % 3;
      delay(150);
    } else if (rawY > 800) {
      shopIndex = (shopIndex + 1) % 3;
      delay(150);
    }

    if (currentSwState == LOW && lastSwState == HIGH) {
      if (shopIndex == 0 && clicks >= upgrade1Cost) {
        clicks -= upgrade1Cost;
        clickPower++;
        upgrade1Cost = (long long)(upgrade1Cost * 1.5);
      } else if (shopIndex == 1 && clicks >= upgrade2Cost) {
        clicks -= upgrade2Cost;
        autoClickers++;
        upgrade2Cost = (long long)(upgrade2Cost * 1.6);
      } else if (shopIndex == 2 && clicks >= upgrade3Cost) {
        clicks -= upgrade3Cost;
        clickMultiplier++;
        upgrade3Cost = (long long)(upgrade3Cost * 2.5);
      }
    }

    display.setCursor(5, 2);
    display.print("- SHOP (J-Up/Dn) -");

    display.setCursor(2, 14);
    if (shopIndex == 0) display.print(">"); else display.print(" ");
    display.print("Pwr+1(");
    display.print(upgrade1Cost);
    display.print(")");

    display.setCursor(2, 26);
    if (shopIndex == 1) display.print(">"); else display.print(" ");
    display.print("Aut+1(");
    display.print(upgrade2Cost);
    display.print(")");

    display.setCursor(2, 38);
    if (shopIndex == 2) display.print(">"); else display.print(" ");
    display.print("Mul+1(");
    display.print(upgrade3Cost);
    display.print(")");

    display.setCursor(5, 52);
    display.print("Bal: ");
    display.print(clicks);
  }

  lastSwState = currentSwState;
  return false;
}

bool pingPongGame() {
  const byte WIDTH = SCREEN_WIDTH;
  const byte HEIGHT = SCREEN_HEIGHT;
  static int ballX = 64, ballY = 32;
  static int ballDirX = 1, ballDirY = 1;
  static int paddleY = 24;
  static int score = 0;
  static unsigned long lastUpdate = 0;
  static bool initialized = false;

  if (!initialized) {
    ballX = 64;
    ballY = 32;
    ballDirX = 1;
    ballDirY = 1;
    paddleY = 24;
    score = 0;
    lastUpdate = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  int rawY = analogRead(PIN_Y);
  paddleY = map(rawY, 0, 1023, 0, HEIGHT - 16);

  if (millis() - lastUpdate > 35) {
    lastUpdate = millis();
    ballX += ballDirX;
    ballY += ballDirY;

    if (ballY <= 0 || ballY >= HEIGHT - 2) ballDirY *= -1;
    if (ballX >= WIDTH - 2) ballDirX *= -1;

    if (ballX <= 6 && ballY >= paddleY && ballY <= paddleY + 16) {
      ballDirX *= -1;
      score++;
      ballX = 7;
    }

    if (ballX < 0) {
      score = 0;
      ballX = 64;
      ballY = 32;
      ballDirX = 1;
    }
  }

  display.clearDisplay();
  display.fillRect(2, paddleY, 3, 16, SSD1306_WHITE);
  display.fillRect(ballX, ballY, 2, 2, SSD1306_WHITE);
  display.setCursor(50, 0);
  display.print(score);
  return false;
}

bool flappyGame() {
  static float birdY = 32;
  static float velocity = 0;
  static int pipeX = 128;
  static int pipeGapY = 24;
  static int score = 0;
  static bool lastSwState = HIGH;
  static bool initialized = false;
  static unsigned long lastTime = 0;

  if (!initialized) {
    birdY = 32;
    velocity = 0;
    pipeX = 128;
    pipeGapY = 24;
    score = 0;
    lastTime = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  bool swState = digitalRead(PIN_SW);
  if (swState == LOW && lastSwState == HIGH) {
    velocity = -2.8;
  }
  lastSwState = swState;

  if (millis() - lastTime > 30) {
    lastTime = millis();
    velocity += 0.35;
    birdY += velocity;

    pipeX -= 2;
    if (pipeX < -12) {
      pipeX = 128;
      pipeGapY = random(10, 40);
      score++;
    }

    if (birdY < 0 || birdY > 60 || (pipeX < 18 && pipeX > 6 && (birdY < pipeGapY || birdY > pipeGapY + 22))) {
      score = 0;
      birdY = 32;
      velocity = 0;
      pipeX = 128;
    }
  }

  display.clearDisplay();
  display.fillRect(10, (int)birdY, 4, 4, SSD1306_WHITE);
  display.fillRect(pipeX, 0, 10, pipeGapY, SSD1306_WHITE);
  display.fillRect(pipeX, pipeGapY + 22, 10, 64 - (pipeGapY + 22), SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(score);
  return false;
}

bool mazeGame() {
  static int playerX = 2, playerY = 2;
  static int score = 0;
  static bool initialized = false;
  static unsigned long moveDelay = 0;

  if (!initialized) {
    playerX = 2;
    playerY = 2;
    score = 0;
    moveDelay = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  if (millis() - moveDelay > 120) {
    moveDelay = millis();
    int xVal = analogRead(PIN_X);
    int yVal = analogRead(PIN_Y);

    if (xVal < 200 && playerX > 1) playerX--;
    else if (xVal > 800 && playerX < 122) playerX++;

    if (yVal < 200 && playerY > 1) playerY--;
    else if (yVal > 800 && playerY < 58) playerY++;

    if (playerX > 115 && playerY > 50) {
      score++;
      playerX = 2;
      playerY = 2;
    }
  }

  display.clearDisplay();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.fillRect(40, 0, 4, 45, SSD1306_WHITE);
  display.fillRect(80, 20, 4, 44, SSD1306_WHITE);
  display.fillRect(playerX, playerY, 3, 3, SSD1306_WHITE);
  display.fillRect(118, 54, 6, 6, SSD1306_WHITE);
  return false;
}

bool dodgeGame() {
  static int playerX = 60;
  static int obsX[3] = { 20, 60, 100 };
  static int obsY[3] = { -10, -30, -50 };
  static int score = 0;
  static bool initialized = false;
  static unsigned long lastTick = 0;

  if (!initialized) {
    playerX = 60;
    score = 0;
    obsY[0] = -10;
    obsY[1] = -30;
    obsY[2] = -50;
    lastTick = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  int xVal = analogRead(PIN_X);
  if (xVal < 200 && playerX > 2) playerX -= 3;
  if (xVal > 800 && playerX < 120) playerX += 3;

  if (millis() - lastTick > 35) {
    lastTick = millis();
    score++;
    for (int i = 0; i < 3; i++) {
      obsY[i] += 2;
      if (obsY[i] > 64) {
        obsY[i] = random(-40, -10);
        obsX[i] = random(5, 115);
      }
      if (abs(playerX - obsX[i]) < 6 && abs(52 - obsY[i]) < 6) {
        score = 0;
        obsY[0] = -10;
        obsY[1] = -30;
        obsY[2] = -50;
      }
    }
  }

  display.clearDisplay();
  display.fillRect(playerX, 52, 6, 8, SSD1306_WHITE);
  for (int i = 0; i < 3; i++) {
    display.fillRect(obsX[i], obsY[i], 6, 6, SSD1306_WHITE);
  }
  display.setCursor(0, 0);
  display.print(score);
  return false;
}

bool dinoGame() {
  static int dinoY = 48;
  static float velocity = 0;
  static int obstacleX = 128;
  static int score = 0;
  static bool lastSwState = HIGH;
  static bool initialized = false;
  static unsigned long lastTime = 0;

  if (!initialized) {
    dinoY = 48;
    velocity = 0;
    obstacleX = 128;
    score = 0;
    lastTime = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  bool swState = digitalRead(PIN_SW);
  if (swState == LOW && lastSwState == HIGH && dinoY == 48) {
    velocity = -4.5;
  }
  lastSwState = swState;

  if (millis() - lastTime > 25) {
    lastTime = millis();
    velocity += 0.4;
    dinoY += velocity;

    if (dinoY > 48) {
      dinoY = 48;
      velocity = 0;
    }

    obstacleX -= 3;
    if (obstacleX < -10) {
      obstacleX = 128;
      score++;
    }

    if (obstacleX < 18 && obstacleX > 8 && dinoY >= 42) {
      score = 0;
      obstacleX = 128;
    }
  }

  display.clearDisplay();
  display.drawFastHLine(0, 55, 128, SSD1306_WHITE);
  display.fillRect(12, dinoY, 6, 8, SSD1306_WHITE);
  display.fillRect(obstacleX, 48, 5, 8, SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(score);
  return false;
}

bool snakeGame() {
  const int SNAKE_SIZE = 4;
  const int MAX_LENGTH = 100;

  static int snakeX[MAX_LENGTH];
  static int snakeY[MAX_LENGTH];
  static int snakeLength = 5;

  static int dirX = 1;
  static int dirY = 0;

  static int foodX = 32;
  static int foodY = 32;

  static unsigned long lastMoveTime = 0;
  const unsigned long moveInterval = 150;
  static bool initialized = false;

  if (!initialized) {
    snakeLength = 5;
    dirX = 1;
    dirY = 0;
    for (int i = 0; i < snakeLength; i++) {
      snakeX[i] = 32 - (i * SNAKE_SIZE);
      snakeY[i] = 32;
    }
    foodX = 64;
    foodY = 32;
    lastMoveTime = millis();
    initialized = true;
  }

  if (digitalRead(PIN_GAME_BTN) == LOW) {
    isGameRunning = false;
    initialized = false;
    delay(200);
    return true;
  }

  int xVal = analogRead(PIN_X);
  int yVal = analogRead(PIN_Y);

  if (xVal < 200 && dirX == 0) {
    dirX = -1;
    dirY = 0;
  } else if (xVal > 800 && dirX == 0) {
    dirX = 1;
    dirY = 0;
  } else if (yVal < 200 && dirY == 0) {
    dirX = 0;
    dirY = -1;
  } else if (yVal > 800 && dirY == 0) {
    dirX = 0;
    dirY = 1;
  }

  if (millis() - lastMoveTime > moveInterval) {
    lastMoveTime = millis();

    for (int i = snakeLength - 1; i > 0; i--) {
      snakeX[i] = snakeX[i - 1];
      snakeY[i] = snakeY[i - 1];
    }

    snakeX[0] += dirX * SNAKE_SIZE;
    snakeY[0] += dirY * SNAKE_SIZE;

    if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
      initialized = false;
      return false;
    }

    for (int i = 1; i < snakeLength; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
        initialized = false;
        return false;
      }
    }

    if (snakeX[0] == foodX && snakeY[0] == foodY) {
      if (snakeLength < MAX_LENGTH) {
        snakeLength++;
      }
      foodX = (random(1, (SCREEN_WIDTH / SNAKE_SIZE) - 1)) * SNAKE_SIZE;
      foodY = (random(1, (SCREEN_HEIGHT / SNAKE_SIZE) - 1)) * SNAKE_SIZE;
    }
  }

  display.clearDisplay();

  for (int i = 0; i < snakeLength; i++) {
    display.fillRect(snakeX[i], snakeY[i], SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);
  }

  display.fillRect(foodX, foodY, SNAKE_SIZE, SNAKE_SIZE, SSD1306_WHITE);

  return false;
}