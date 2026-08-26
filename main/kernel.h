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

  void sortAllTasks() {

    Task* allTasks[16];
    uint8_t total = 0;

    for (uint8_t i = 0; i < count; i++) {
      if (tasks[i] != nullptr) allTasks[total++] = tasks[i];
    }
    for (uint8_t i = 0; i < sleepCount; i++) {
      if (sleeping[i] != nullptr) allTasks[total++] = sleeping[i];
    }

    if (total <= 1) return;

    for (uint8_t i = 0; i < total - 1; i++) {
      for (uint8_t j = 0; j < total - i - 1; j++) {
        if (allTasks[j]->getPriority() < allTasks[j + 1]->getPriority()) {
          Task* temp = allTasks[j];
          allTasks[j] = allTasks[j + 1];
          allTasks[j + 1] = temp;
        }
      }
    }

    count = 0;
    sleepCount = 0;
    for (uint8_t i = 0; i < 4; i++) tasks[i] = nullptr;
    for (uint8_t i = 0; i < 12; i++) sleeping[i] = nullptr;

    for (uint8_t i = 0; i < total; i++) {
      if (i < 4) {
        tasks[count] = allTasks[i];
        tasks[count]->state = TaskState::READY;
        count++;
      } else {
        if (sleepCount < 12) {
          sleeping[sleepCount] = allTasks[i];
          sleeping[sleepCount]->state = TaskState::READY;
          sleepCount++;
        }
      }
    }
    index = count;
  }

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
      t->state = TaskState::READY;
      tasks[count] = t;
      count++;
    } else if (sleepCount < 12) {
      t->state = TaskState::PAUSED;
      sleeping[sleepCount] = t;
      sleepCount++;
    }

    sortAllTasks();
  }

  void reset() {
    index = 0;
    count = 0;
    sleepCount = 0;
    for (uint8_t i = 0; i < 4; i++) tasks[i] = nullptr;
    for (uint8_t i = 0; i < 12; i++) sleeping[i] = nullptr;
  }

  void sortTask() {
    sortAllTasks();
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

#endif  // kernel.h