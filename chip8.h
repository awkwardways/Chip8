#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>

#define MEMORY_SIZE          4096
#define STACK_SIZE           64
#define CHIP_8_SCREEN_WIDTH  64
#define CHIP_8_SCREEN_HEIGHT 32

extern uint8_t mainMemory[MEMORY_SIZE];
extern uint16_t stack[STACK_SIZE], PC, I;
extern uint8_t reg[16];
extern uint8_t SP, delayTimer, soundTimer;
extern uint32_t newScreen[CHIP_8_SCREEN_HEIGHT * CHIP_8_SCREEN_WIDTH];
extern int drawFlag;
extern int waitingForKeypress;
extern uint8_t keyPressed;
void initC8();
int loadC8(const char* romName);
int cycle();
#endif
