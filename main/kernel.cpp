#include "kernel.h"

void Kernel::run() {
  sortTask();
  for (uint8_t i = 0; i < count; i++) {
    if (tasks[i] != nullptr && (tasks[i]->state == TaskState::READY || tasks[i]->state == TaskState::RUNNING)) {
      tasks[i]->state = TaskState::RUNNING;

      bool isDone = tasks[i]->execute();

      if (isDone) {
        freeTask(i);
        i--;
        sortTask();
      }
    }
  }
  sortTask();
}

void Kernel::freeTask(uint8_t indexToFree) {
  if (indexToFree >= count) return;

  if (tasks[indexToFree] != nullptr) {
    delete tasks[indexToFree];
    tasks[indexToFree] = nullptr;
  }

  for (uint8_t i = indexToFree; i < count - 1; i++) {
    tasks[i] = tasks[i + 1];
  }
  tasks[count - 1] = nullptr;

  count--;
  index = count;

  if (sleepCount > 0 && count < 4) {
    Task* nextTask = sleeping[0];
    for (uint8_t i = 0; i < sleepCount - 1; i++) {
      sleeping[i] = sleeping[i + 1];
    }
    sleeping[sleepCount - 1] = nullptr;
    sleepCount--;
    addTask(nextTask);
  }
}  //kernel.cpp