#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include "cpu.h"
#include "task.h"

class Kernel {
private:
  Task* tasks[4];
  Task* sleeping[12];

  uint8_t count;
  uint8_t sleepCount;
  CPU* core;
  uint8_t index;

public:
  Kernel(CPU& cpuCore) {
    index = 0;
    count = 0;
    sleepCount = 0;
    core = &cpuCore;
    for (uint8_t i = 0; i < 4; i++) tasks[i] = nullptr;
    for (uint8_t i = 0; i < 12; i++) sleeping[i] = nullptr;
  }

  void addTask(Task* t) {

    if (t == nullptr) return;

    if (count < 4) {
      tasks[count] = t;
      tasks[count]->state = TaskState::READY;
      count++;
      index = count;
    } else {
      if (sleepCount < 12) {
        sleeping[sleepCount] = t;
        sleeping[sleepCount]->state = TaskState::READY;
        sleepCount++;
      }
    }
  }

  void reset() {
    index = 0;
    count = 0;
    sleepCount = 0;
    for (uint8_t i = 0; i < 4; i++) tasks[i] = nullptr;
    for (uint8_t i = 0; i < 12; i++) sleeping[i] = nullptr;
  }

  void sortTask() {
    if (count <= 1) return;
    for (uint8_t i = 0; i < count - 1; i++) {
      for (uint8_t j = 0; j < count - i - 1; j++) {
        if (tasks[j]->getPriority() < tasks[j + 1]->getPriority()) {

          Task* temp = tasks[j];
          tasks[j] = tasks[j + 1];
          tasks[j + 1] = temp;
        }
      }
    }
  }

  void run();
  void freeTask(uint8_t indexToFree);

  uint8_t getCount() {
    return count;
  }
  uint8_t getSleepCount() {
    return sleepCount;
  }

  Task& getTask(uint8_t i) {
    static Task emptyTask;
    if (i < 4 && tasks[i] != nullptr) {
      return *tasks[i];
    }
    return emptyTask;
  }
};

#endif  //kernel.h