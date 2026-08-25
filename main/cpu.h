#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <cstring>

class CPU {
private:
  static uint16_t* pool;
  static bool blocks[4];
  static bool sleeps[12];

public:
  static uint16_t getUsed();
  static uint16_t* sell(uint16_t size);
  static void freeMemory(uint16_t* ptr);
};

#endif  //cpu.h