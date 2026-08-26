#include "kernel.h"

void Kernel::run() {
  sortTask();

  for (uint8_t i = 0; i < count; i++) {
    if (tasks[i] != nullptr && (tasks[i]->state == TaskState::READY || tasks[i]->state == TaskState::RUNNING)) {

      tasks[i]->state = TaskState::RUNNING;

      bool isDone1 = false;
      bool isDone2 = false;
      bool isDone3 = false;
      bool isDone4 = false;

      if (i == 0 && count >= 4 && tasks[0] != nullptr && tasks[1] != nullptr && tasks[2] != nullptr && tasks[3] != nullptr) {

        isDone1 = tasks[0]->execute();
        isDone2 = tasks[1]->execute();
        isDone3 = tasks[2]->execute();
        isDone4 = tasks[3]->execute();

        tasks[0]->state = TaskState::RUNNING;
        tasks[1]->state = TaskState::RUNNING;
        tasks[2]->state = TaskState::RUNNING;
        tasks[3]->state = TaskState::RUNNING;

        if (isDone1) {
          freeTask(0);
          sortTask();
        }
        if (isDone2) {
          freeTask(1);
          sortTask();
        }
        if (isDone3) {
          freeTask(2);
          sortTask();
        }
        if (isDone4) {
          freeTask(3);
          sortTask();
        }
      } else if (i == 0 && count >= 4 && tasks[0] != nullptr && tasks[1] != nullptr && tasks[2] != nullptr) {
        isDone1 = tasks[0]->execute();
        isDone2 = tasks[1]->execute();
        isDone3 = tasks[2]->execute();

        tasks[0]->state = TaskState::RUNNING;
        tasks[1]->state = TaskState::RUNNING;
        tasks[2]->state = TaskState::RUNNING;

        if (isDone1) {
          freeTask(0);
          sortTask();
        }
        if (isDone2) {
          freeTask(1);
          sortTask();
        }
        if (isDone3) {
          freeTask(2);
          sortTask();
        }
      } else if (i == 0 && count >= 4 && tasks[0] != nullptr && tasks[1] != nullptr) {
        isDone1 = tasks[0]->execute();
        isDone2 = tasks[1]->execute();

        tasks[0]->state = TaskState::RUNNING;
        tasks[1]->state = TaskState::RUNNING;

        if (isDone1) {
          freeTask(0);
          sortTask();
        }
        if (isDone2) {
          freeTask(1);
          sortTask();
        }
      } else if (i == 0 && count >= 4) {
        isDone1 = tasks[0]->execute();

        tasks[0]->state = TaskState::RUNNING;

        if (isDone1) {
          freeTask(0);
          sortTask();
        }
      } else {
        bool isDone = tasks[i]->execute();
        if (isDone) {
          tasks[i]->state = TaskState::DONE;
          freeTask(i);
          i--;
          sortTask();
        }
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
    nextTask->state = TaskState::READY;
    for (uint8_t i = 0; i < sleepCount - 1; i++) {
      sleeping[i] = sleeping[i + 1];
    }
    sleeping[sleepCount - 1] = nullptr;
    sleepCount--;
    addTask(nextTask);
  }
}  //kernel.cpp