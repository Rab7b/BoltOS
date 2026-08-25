#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <new>
#include "cpu.h"

enum class TaskState {
  READY,
  RUNNING,
  PAUSED,
  DONE
};

class Task {
private:
  uint8_t priority;
  bool (*taskFunc)();
  void* space;

public:
  TaskState state;

  Task() {
    priority = 0;
    taskFunc = nullptr;
    space = nullptr;
    state = TaskState::READY;
  }

  Task(uint8_t p, TaskState s, bool (*func)()) {
    priority = p;
    state = s;
    taskFunc = func;
    space = nullptr;
  }

  void* operator new(size_t size) {
    void* ptr = CPU::sell(size);
    if (!ptr) {
      return nullptr;
    }
    return ptr;
  }

  void operator delete(void* ptr) {
    if (ptr != nullptr) {
      CPU::freeMemory(static_cast<uint16_t*>(ptr));
    }
  }

  void setPriority(uint8_t p) {
    priority = p;
  }
  void setTaskFunc(bool (*func)()) {
    taskFunc = func;
  }

  uint8_t getPriority() const {
    return priority;
  }
  bool (*getTaskFunc() const)() {
    return taskFunc;
  }

  void* getSpace() const {
    return space;
  }
  void setSpace(void* ptr) {
    space = ptr;
  }

  bool execute() {
    if (taskFunc != nullptr) {
      return taskFunc();
    }
    return true;
  }
};

#endif  //task.h