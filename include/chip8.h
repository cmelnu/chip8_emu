#ifndef CHIP8_H
#define CHIP8_H

#include <time.h>
#include <stddef.h> //for size_t
#include "config.h"
#include "memory.h"
#include "registers.h"
#include "stack.h"
#include "keyboard.h"
#include "screen.h"

struct chip8 {
  struct chip8_memory memory;
  struct chip8_stack stack;
  struct chip8_registers registers;
  struct chip8_keyboard keyboard;
  struct chip8_screen screen;
};

void chip8_init (struct chip8* chip);
void chip8_load (struct chip8* chip8, const char* buf, size_t size);
void chip8_exec (struct chip8* chip8, unsigned short opcode);

#endif
