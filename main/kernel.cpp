#include "kernel.h"

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

        Task* t0 = tasks[0];
        Task* t1 = tasks[1];
        Task* t2 = tasks[2];
        Task* t3 = tasks[3];

        isDone1 = t0->execute();
        isDone2 = t1->execute();
        isDone3 = t2->execute();
        isDone4 = t3->execute();

        t0->state = TaskState::RUNNING;
        t1->state = TaskState::RUNNING;
        t2->state = TaskState::RUNNING;
        t3->state = TaskState::RUNNING;

        if (isDone1) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t0) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone2) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t1) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone3) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t2) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone4) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t3) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }

      } else if (i == 0 && count >= 3 && tasks[0] != nullptr && tasks[1] != nullptr && tasks[2] != nullptr) {  
        Task* t0 = tasks[0];
        Task* t1 = tasks[1];
        Task* t2 = tasks[2];

        isDone1 = t0->execute();
        isDone2 = t1->execute();
        isDone3 = t2->execute();

        t0->state = TaskState::RUNNING;
        t1->state = TaskState::RUNNING;
        t2->state = TaskState::RUNNING;

        if (isDone1) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t0) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone2) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t1) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone3) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t2) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }

      } else if (i == 0 && count >= 2 && tasks[0] != nullptr && tasks[1] != nullptr) {  
        Task* t0 = tasks[0];
        Task* t1 = tasks[1];

        isDone1 = t0->execute();
        isDone2 = t1->execute();

        t0->state = TaskState::RUNNING;
        t1->state = TaskState::RUNNING;

        if (isDone1) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t0) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }
        if (isDone2) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t1) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }

      } else if (i == 0 && count >= 1 && tasks[0] != nullptr) {  
        Task* t0 = tasks[0];
        isDone1 = t0->execute();
        t0->state = TaskState::RUNNING;

        if (isDone1) {
          for (uint8_t k = 0; k < count; k++) {
            if (tasks[k] == t0) {
              freeTask(k);
              break;
            }
          }
          sortTask();
        }

      } else {
        if (tasks[i] != nullptr) {
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

  if (sleepCount > 0 && count < 4) {
    Task* nextTask = sleeping[0];
    nextTask->state = TaskState::READY;

    for (uint8_t i = 0; i < sleepCount - 1; i++) {
      sleeping[i] = sleeping[i + 1];
    }
    sleeping[sleepCount - 1] = nullptr;
    sleepCount--;

    tasks[count] = nextTask;
    count++;
  }
}  //kernel.cpp