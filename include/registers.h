#ifndef REGISTERS_H
#define REGISTERS_H

#include "config.h"

struct chip8_registers {
  unsigned char V[CHIP8_TOTAL_DATA_REGISTERS];
  unsigned char I;
  unsigned char delay_timer;
  unsigned char sound_timer;
  unsigned short PC;
  unsigned char SP;
};

#endif
