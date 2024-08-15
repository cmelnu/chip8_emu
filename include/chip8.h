#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"
#include "memory.h"
#include "registers.h"

struct chip8 {
  struct chip8_memory memory;
  struct chip8_registers registers;
};

#endif
