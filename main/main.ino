#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "cpu.h"
#include "task.h"
#include "kernel.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

CPU myCpu;
Kernel osKernel(myCpu);

enum class SystemState {
  DESKTOP,
  TASK_MANAGER,
  SETTINGS
};

SystemState currentScreen = SystemState::DESKTOP;

int cursorX = 64;
int cursorY = 32;
int cursorSpeed = 2;

const uint8_t JOY_X = A1;
const uint8_t JOY_Y = A2;
const uint8_t JOY_SW = 2;

const int icon1X = 20;
const int icon1Y = 24;
const int iconSize = 10; 

const int icon2X = 80;
const int icon2Y = 24;

void drawDesktop();
void drawTaskManager();
void drawSettings();

bool sampleTask() {
  static unsigned long randomDuration{ 0 };
  static unsigned long startTime{ 0 };
  static bool initialized{ false };

  if (!initialized) {
    randomDuration = random(1000, 3001);
    startTime = millis();
    initialized = true;
  }

  if (millis() - startTime >= randomDuration) {
    initialized = false;
    return true;
  }
  return false;
}

bool taskManagerApp() {
  return currentScreen != SystemState::TASK_MANAGER;
}

bool settingsApp() {
  return currentScreen != SystemState::SETTINGS;
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  pinMode(JOY_SW, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {}
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28, 24);
  display.print(F("BoltOS"));
  display.display();

  delay(1200);

  display.setTextSize(1);
}

void loop() {
  osKernel.run();

  display.clearDisplay();

  int joyX = analogRead(JOY_X); 
  int joyY = analogRead(JOY_Y); 

  if (joyX < 400) cursorX -= cursorSpeed;
  if (joyX > 600) cursorX += cursorSpeed;
  if (joyY < 400) cursorY -= cursorSpeed;
  if (joyY > 600) cursorY += cursorSpeed;

  if (cursorX < 0) cursorX = 0;
  if (cursorX > 124) cursorX = 124;
  if (cursorY < 12) cursorY = 12;
  if (cursorY > 60) cursorY = 60;

  switch (currentScreen) {
    case SystemState::DESKTOP:
      drawDesktop();
      break;
    case SystemState::TASK_MANAGER:
      drawTaskManager();
      break;
    case SystemState::SETTINGS:
      drawSettings();
      break;
  }

  display.display();
  delay(30); 
}

void drawDesktop() {
  display.setCursor(0, 0);
  display.print(F("== BOLT OS Desktop =="));
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  display.drawRect(icon1X, icon1Y, iconSize, iconSize, SSD1306_WHITE);
  display.drawFastHLine(icon1X + 2, icon1Y + 3, 6, SSD1306_WHITE);
  display.drawFastHLine(icon1X + 2, icon1Y + 5, 6, SSD1306_WHITE);
  display.drawFastHLine(icon1X + 2, icon1Y + 7, 6, SSD1306_WHITE);

  display.setCursor(icon1X - 6, icon1Y + 12);
  display.print(F("Tasks"));

  display.drawRect(icon2X, icon2Y, iconSize, iconSize, SSD1306_WHITE);
  display.drawPixel(icon2X + 5, icon2Y + 3, SSD1306_WHITE);
  display.drawPixel(icon2X + 5, icon2Y + 6, SSD1306_WHITE);
  display.drawPixel(icon2X + 3, icon2Y + 4, SSD1306_WHITE);
  display.drawPixel(icon2X + 7, icon2Y + 4, SSD1306_WHITE);

  display.setCursor(icon2X - 6, icon2Y + 12);
  display.print(F("Setup"));

  display.setCursor(cursorX, cursorY);
  display.print(F("^"));

  if (digitalRead(JOY_SW) == LOW) {
    if (cursorX >= icon1X - 4 && cursorX <= icon1X + iconSize + 4 &&
        cursorY >= icon1Y - 4 && cursorY <= icon1Y + iconSize + 14) {
      currentScreen = SystemState::TASK_MANAGER;
      osKernel.addTask(new Task(5, TaskState::READY, taskManagerApp));
      delay(300);
    }
    else if (cursorX >= icon2X - 4 && cursorX <= icon2X + iconSize + 4 &&
             cursorY >= icon2Y - 4 && cursorY <= icon2Y + iconSize + 14) {
      currentScreen = SystemState::SETTINGS;
      osKernel.addTask(new Task(3, TaskState::READY, settingsApp));
      delay(300);
    }
  }
}

void drawTaskManager() {
  display.setCursor(0, 0);
  display.print(F("=== TASK MANAGER ==="));

  double memoryUsed{ (static_cast<double>(CPU::getUsed()) / 1024.0) * 100.0 };
  display.setCursor(0, 10);
  display.print(F("RAM: "));
  display.print(memoryUsed, 1);
  display.print(F("%"));

  display.setCursor(72, 10);
  display.print(F("Sleep:"));
  display.print(osKernel.getSleepCount());

  display.drawFastHLine(0, 20, 128, SSD1306_WHITE);

  for (uint8_t i = 0; i < 4; i++) {
    display.setCursor(0, 24 + (i * 10));
    display.print(i + 1);
    display.print(F("> "));

    const Task& currentTask = osKernel.getTask(i);

    if (currentTask.getTaskFunc() != nullptr) {
      display.print(F("P"));
      display.print(currentTask.getPriority());
      display.print(F(" "));

      switch (currentTask.state) {
        case TaskState::READY:   display.print(F("READY"));   break;
        case TaskState::RUNNING: display.print(F("RUNNING")); break;
        case TaskState::PAUSED:  display.print(F("PAUSED"));  break;
        case TaskState::DONE:    display.print(F("DONE"));    break;
      }
    } else {
      display.print(F("[ EMPTY ]"));
    }
  }

  if (digitalRead(JOY_SW) == LOW) {
    currentScreen = SystemState::DESKTOP;
    delay(300);
  }
}

void drawSettings() {
  display.setCursor(0, 0);
  display.print(F("==== SETTINGS ===="));
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  display.setCursor(10, 22);
  display.print(F("Cursor Speed: "));
  display.print(cursorSpeed);

  display.setCursor(10, 36);
  display.print(F("Push Y-Up/Down"));
  display.setCursor(10, 46);
  display.print(F("to change speed!"));

  int joyY = analogRead(JOY_Y);
  static unsigned long lastChange = 0;
  if (millis() - lastChange > 250) {
    if (joyY < 400 && cursorSpeed < 5) {
      cursorSpeed++;
      lastChange = millis();
    }
    else if (joyY > 600 && cursorSpeed > 1) {
      cursorSpeed--;
      lastChange = millis();
    }
  }

  display.setCursor(10, 56);
  display.print(F("Click SW to Exit"));

  if (digitalRead(JOY_SW) == LOW) {
    currentScreen = SystemState::DESKTOP;
    delay(300);
  }
}