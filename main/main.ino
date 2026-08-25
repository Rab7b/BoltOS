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

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  osKernel.addTask(new Task(1, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(2, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(3, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(4, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(5, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(6, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(7, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(8, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(9, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(10, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(11, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(12, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(13, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(14, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(15, TaskState::READY, sampleTask));
  osKernel.addTask(new Task(16, TaskState::READY, sampleTask));
}

void loop() {
  osKernel.run();

  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(F("====== BOLT OS ======"));

  double memoryUsed{ (static_cast<double>(CPU::getUsed()) / 1024.0) * 100.0 };

  display.setCursor(0, 10);
  display.print(F("RAM:"));
  display.print(memoryUsed);
  display.print(F("%"));

  display.setCursor(72, 13);
  display.print(F("Sleep: "));
  display.print(osKernel.getSleepCount());

  display.drawFastHLine(0, 22, 128, SSD1306_WHITE);

  for (uint8_t i = 0; i < 4; i++) {
    display.setCursor(0, 28 + (i * 9));
    display.print(i + 1);
    display.print(F(": "));

    const Task& currentTask = osKernel.getTask(i);

    if (currentTask.getTaskFunc() != nullptr) {
      display.print(F("P"));
      display.print(currentTask.getPriority());
      display.print(F(" "));

      switch (currentTask.state) {
        case TaskState::READY: display.print(F("READY")); break;
        case TaskState::RUNNING: display.print(F("RUNNING")); break;
        case TaskState::PAUSED: display.print(F("PAUSED")); break;
        case TaskState::DONE: display.print(F("DONE")); break;
      }
    } else {
      display.print(F("[ EMPTY ]"));
    }
  }

  display.display();
  delay(100);
}  //main.ino