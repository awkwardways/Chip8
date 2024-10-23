#include "chip8.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#define SCREEN_COORDS(x , y) (x + shift) + (y + height) * CHIP_8_SCREEN_WIDTH
#define HEX_TO_ASCII(c) (c + ((c > 9)?87:48))

uint8_t mainMemory[MEMORY_SIZE];
uint16_t stack[STACK_SIZE], PC, I;
uint8_t reg[16];
uint8_t SP, delayTimer, soundTimer;
uint8_t keyPressed;
int waitingForKeypress;
uint32_t newScreen[CHIP_8_SCREEN_HEIGHT * CHIP_8_SCREEN_WIDTH];
int drawFlag;
uint32_t pixelState[2] = {0x00000000, 0xffffff00};

void initC8() {
	srand(time(NULL));
	memset(mainMemory, 0, MEMORY_SIZE);
	memset(stack, 0, 2*STACK_SIZE);
	memset(reg, 0, 16);
	memset(newScreen, 0, sizeof(newScreen));
	PC = 0x200;
	SP = 0;
	waitingForKeypress = 0;
	keyPressed = 16;
}

int loadC8(const char* romName) {
 FILE* src = fopen(romName, "rb");	
 if(src == NULL) {
		printf("Could not open ROM file.\n");
		return 1;
 }
 fread(&mainMemory[0x200], sizeof(uint8_t), MEMORY_SIZE - 0x200, src);
 fclose(src);
 return 0;
}

int cycle() {
	uint16_t instruction = (mainMemory[PC] & 0xf0) << 8;
	uint16_t address = (mainMemory[PC] & 0x0f) << 8 | (mainMemory[PC + 1]);  
	uint8_t byte = mainMemory[PC + 1];
	uint8_t regx = mainMemory[PC] & 0x0f;
	uint8_t regy = (mainMemory[PC + 1] & 0xf0) >> 4;
	uint8_t spriteHeight = mainMemory[PC + 1] & 0x0f;

	switch(instruction) {
		case 0x0000:
			if(byte == 0xe0) {
				memset(newScreen, 0, sizeof(newScreen));
				PC += 2;
			} 
			else if(byte == 0xee) {
				PC = stack[--SP];
			} 
		break;

		case 0x1000:
			PC = address;
		break;

		case 0x2000:
			stack[SP++] = (PC + 2);
			PC = address;
		break;
		
		case 0x3000:
			PC += 2 + ((reg[regx] == byte) << 1);
		break;

		case 0x4000:
			PC += 2 + ((reg[regx] != byte) << 1);
		break;

		case 0x5000:
			PC += 2 + ((reg[regx] == reg[regy]) << 1);
		break;
		
		case 0x6000:
			reg[regx] = byte;
			PC += 2;
		break;

		case 0x7000:
			reg[regx] += byte;
			PC += 2;
		break;

		case 0x8000:
			switch(byte & 0x0f) {
				case 0x0:
					reg[regx] = reg[regy];
				break;

				case 0x1:
					reg[regx] |= reg[regy];
				break;

				case 0x2:
					reg[regx] &= reg[regy];
				break;

				case 0x3:
					reg[regx] ^= reg[regy];
				break;

				case 0x4:
					stack[SP] = reg[regx];
					reg[regx] += reg[regy];
					if(stack[SP] + reg[regy] > 255) reg[0xf] = 1;
					else reg[0xf] = 0;
				break; 

				case 0x5:
					stack[SP] = reg[regx];
					reg[regx] -= reg[regy];
					if(reg[regy] > stack[SP]) reg[0xf] = 0;
					else reg[0xf] = 1;
				break;

				case 0x6:
					stack[SP] = reg[regx];
					reg[regx] >>= 1;
					reg[0xf] = stack[SP] & 0x1;

				break;

				case 0x7:
					reg[regx] = reg[regy] - reg[regx];
					if(reg[regy] >= reg[regx]) reg[0xf] = 1;
					else reg[0xf] = 0;
				break;

				case 0xe:
					stack[SP] = reg[regx];
					reg[regx] <<= 1;
					reg[0xf] = (stack[SP] & 0x80) >> 7;
				break;
			}
			PC += 2;
		break;

		case 0x9000:
			PC += 2 + ((reg[regx] != reg[regy]) << 1);
		break;
		
		case 0xa000:
			I = address;
			PC += 2;
		break;

		case 0xb000:
			PC = reg[0] + address;
		break;

		case 0xc000:
			reg[regx] = rand() & byte;
			PC += 2;
		break;

		case 0xd000:
			drawFlag = 1;
			for(uint8_t height = 0; height < spriteHeight; height++) {
				for(uint8_t shift = 0; shift < 8; shift++) {
					if(newScreen[SCREEN_COORDS(reg[regx], reg[regy])] && ((mainMemory[I + height] >> (7 - shift)) & 0x1)) reg[0xf] = 1;
					newScreen[SCREEN_COORDS(reg[regx], reg[regy])] ^= pixelState[(mainMemory[I + height] >> (7 - shift) & 0x1)]; 
				}
			}
			PC += 2;
		break;

		case 0xe000:
			switch(byte) {
				case 0x9e:
					PC += 2 + ((reg[regx] == keyPressed) << 1);
				break;

				case 0xa1:
					PC += 2 + ((reg[regx] != keyPressed) << 1);
				break;
			}
		break;

		case 0xf000:
			switch(byte) {
				case 0x07:
					reg[regx] = delayTimer;
					PC += 2;
				break;

				case 0x0a: // wait for keypress and store it in vx
					if(keyPressed != 16) {
						reg[regx] = keyPressed;
						PC += 2;
						keyPressed = 16;
					}
				break;

				case 0x15:
					delayTimer = reg[regx];
					PC += 2;
				break;

				case 0x18:
					soundTimer = reg[regx];
					PC += 2;
				break;

				case 0x1e:
					I += reg[regx];
					PC += 2;
				break;

				case 0x29:
					I = reg[regx] * 5;
					PC += 2;
				break;

				case 0x33:
					mainMemory[I] = reg[regx] / 100;
					mainMemory[I + 1] = (reg[regx] % 100) / 10;
					mainMemory[I + 2] = reg[regx] % 10;
					PC += 2;
				break;

				case 0x55:
					for(uint8_t offset = 0; offset <= regx; offset++) {
						mainMemory[I + offset] = reg[offset];
					}
					PC += 2;
				break;

				case 0x65:
					for(uint8_t offset = 0; offset <= regx; offset++) {
						reg[offset] = mainMemory[I + offset];
					}
					PC += 2;
				break;
			}
		break;
	}

}
