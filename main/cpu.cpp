#include "cpu.h"


uint16_t* CPU::pool = new uint16_t[512];
bool CPU::blocks[4] = { false, false, false, false };
bool CPU::sleeps[12] = { false, false, false, false, false, false, false, false, false, false, false, false };

uint16_t* CPU::sell(uint16_t size) {

  for (uint8_t i = 0; i < 4; i++) {
    if (!blocks[i]) {
      blocks[i] = true;
      return &pool[i * 32];
    }
  }

  for (uint8_t i = 0; i < 12; i++) {
    if (!sleeps[i]) {
      sleeps[i] = true;
      return &pool[(4 + i) * 32];
    }
  }

  return nullptr;
}

void CPU::freeMemory(uint16_t* ptr) {
  if (ptr == nullptr) return;

  if (ptr >= pool && ptr < pool + 512) {
    size_t offset = ptr - pool;
    uint16_t blockIndex = offset / 32;

    if (blockIndex < 4) {
      blocks[blockIndex] = false;
      std::memset(&pool[blockIndex * 32], 0, 32 * sizeof(uint16_t));
    } else if (blockIndex >= 4 && blockIndex < 16) {
      uint8_t sleepIndex = blockIndex - 4;
      sleeps[sleepIndex] = false;
      std::memset(&pool[blockIndex * 32], 0, 32 * sizeof(uint16_t));
    }
  }
}

uint16_t CPU::getUsed() {
  uint16_t usedCount = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (blocks[i]) {
      usedCount += 64;
    }
  }
  return usedCount;
}  //cpu.cpp