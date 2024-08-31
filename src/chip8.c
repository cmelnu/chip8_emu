#include "chip8.h"
#include <SDL2/SDL_events.h>
#include <assert.h>
#include <string.h>  // Include this for memset
#include <memory.h>
#include <SDL2/SDL.h>

const char chip8_default_character_set[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xf0, 0x10, 0xf0, 0x80, 0xf0,
    0xf0, 0x10, 0xf0, 0x10, 0xf0,
    0x90, 0x90, 0xf0, 0x10, 0x10,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0xf0, 0x90, 0xf0,
    0xf0, 0x10, 0x20, 0x40, 0x40,
    0xf0, 0x90, 0xf0, 0x90, 0xf0,
    0xf0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x90, 0xf0, 0x90, 0x90,
    0xe0, 0x90, 0xe0, 0x90, 0xe0,
    0xf0, 0x80, 0x80, 0x80, 0xf0,
    0xe0, 0x90, 0x90, 0x90, 0xe0,
    0xf0, 0x80, 0xf0, 0x80, 0xf0,
    0xf0, 0x80, 0xf0, 0x80, 0x80
};

void chip8_init (struct chip8* chip8)
{
    memset(chip8, 0, sizeof(struct chip8));
    memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

void chip8_load (struct chip8* chip8, const char* buf, size_t size)
{
    assert (size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
    memcpy (&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static void chip8_exec_extended_eight (struct chip8* chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000F;
    unsigned char y = (opcode >> 4) & 0x000F;
    unsigned char final_four_bits = opcode & 0x000F;  //They contain the operation we need to run
    unsigned short tmp = 0;

    switch (final_four_bits)
    {

        // LD Vx, Vy (8xy0): Set Vx = Vy.
        case 0x00:
            chip8->registers.V[x] = chip8->registers.V[y];
        break;

        // OR Vx, Vy (8xy1): Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
        case 0x01:
            chip8->registers.V[x] = chip8->registers.V[x] | chip8->registers.V[y];
        break;

        // AND Vx, Vy (8xy2): Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
        case 0x02:
            chip8->registers.V[x] = chip8->registers.V[x] & chip8->registers.V[y];
        break;

        // XOR Vx, Vy (8xy3): Performs a bitwise XOR on the values of Vx and Vy, then stores the result in Vx.
        case 0x03:
            chip8->registers.V[x] = chip8->registers.V[x] ^ chip8->registers.V[y];
        break;

        // ADD Vx, Vy (8xy4): Set Vx = Vx + Vy, set VF = carry.
        case 0x04:
        {
            tmp = chip8->registers.V[x] + chip8->registers.V[y];
            chip8->registers.V[0x0f] = false;
            if (tmp > 0xff)
            {
                chip8->registers.V[0x0f] = true;
            }

            chip8->registers.V[x] = tmp;
        }
        break;

        // SUB Vx, Vy (8xy5): Set Vx = Vx - Vy, set VF = NOT borrow.
        case 0x05:
        {
            chip8->registers.V[0x0f] = false;
            if (chip8->registers.V[x] > chip8->registers.V[y])
            {
                chip8->registers.V[0x0f] = true;
            }
            chip8->registers.V[x] = chip8->registers.V[x] - chip8->registers.V[y];
        }
        break;

        // SHR Vx {, Vy} (8xy6): Set Vx = Vx SHR 1.
        case 0x06:
        {
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0x01;
            chip8->registers.V[x] = chip8->registers.V[x] / 2;
        }
        break;

        // SUBN Vx, Vy (8xy7): Set Vx = Vy - Vx, set VF = NOT borrow.
        case 0x07:
        {
            chip8->registers.V[0x0f] = chip8->registers.V[y] > chip8->registers.V[x];
            chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
        }
        break;

        // SHL Vx {, Vy} (8xyE): Set Vx = Vx SHL 1.
        case 0x0E:
        {
            chip8->registers.V[0x0f] = chip8->registers.V[x] & 0b10000000;
            chip8->registers.V[x] = chip8->registers.V[x] * 2;
        }
        break;

    }
}

static char chip8_wait_for_key_press (struct chip8* chip8)
{
    SDL_Event event;

    while(SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;

        char chip8_key = chip8_keyboard_map (&chip8->keyboard, c);

        if (chip8_key != -1)
        {
            return chip8_key;
        }

    }

    return -1;
}

static void chip8_exec_extended_F(struct chip8* chip8, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000F;

    switch (opcode & 0x00FF)
    {
        // LD Vx, DT (Fx07): Set Vx = delay timer value.
        case 0x07:
            chip8->registers.V[x] = chip8->registers.delay_timer;
        break;

        // LD Vx, K (Fx0A): Wait for a key press, store the value of the key in Vx.
        case 0x0A:
            chip8->registers.V[x] = chip8_wait_for_key_press(chip8);
        break;

        // LD DT, Vx (Fx15): Set delay timer = Vx.
        case 0x15:
            chip8->registers.delay_timer = chip8->registers.V[x];
        break;

        // LD ST, Vx (Fx18): Set sound timer = Vx.
        case 0x18:
            chip8->registers.sound_timer = chip8->registers.V[x];
        break;

        // ADD I, Vx (Fx1E): Set I = I + Vx.
        case 0x1E:
            chip8->registers.I += chip8->registers.V[x];
        break;

        // LD F, Vx (Fx29): Set I = location of sprite for digit Vx.
        case 0x29:
            chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
        break;

        // LD B, Vx (Fx33): Store BCD representation of Vx in memory locations I, I+1, and I+2.
        case 0x33:
            {
                unsigned char hundreds = chip8->registers.V[x] / 100;
                unsigned char tens = (chip8->registers.V[x] / 10) % 10;
                unsigned char ones = chip8->registers.V[x] % 10;
                chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
                chip8_memory_set(&chip8->memory, chip8->registers.I + 1, tens);
                chip8_memory_set(&chip8->memory, chip8->registers.I + 2, ones);
            }
        break;

        // LD [I], Vx (Fx55): Store registers V0 through Vx in memory starting at location I.
        case 0x55:
            for (int i = 0; i <= x; i++)
            {
                chip8_memory_set(&chip8->memory, chip8->registers.I + i, chip8->registers.V[i]);
            }
            break;

        // LD Vx, [I] (Fx65): Read registers V0 through Vx from memory starting at location I.
        case 0x65:
            for (int i = 0; i <= x; i++)
            {
                chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I + i);
            }
            break;
    }
}


static void chip8_exec_extended (struct chip8* chip8, unsigned short opcode)
{
    unsigned short nnn = opcode & 0x0FFF;
    unsigned char x = (opcode >> 8) & 0x000F;
    unsigned char y = (opcode >> 4) & 0x000F;
    unsigned char kk = opcode & 0x00FF;
    unsigned char n = opcode & 0x000F;

    switch (opcode & 0xF000)
    {
        //JP addr (1nnn): Jump to location nnn
        case 0x1000:
            chip8->registers.PC = nnn;
        break;

        //CALL addr (2nnn): Call subroutine at location nnn
        case 0x2000:
            chip8_stack_push (chip8, chip8->registers.PC);
            chip8->registers.PC = nnn;
        break;

        //SE Vx, byte (3xkk): Skip next instruction if Vx = kk
        case 0x3000:
            if (chip8->registers.V[x] == kk)
            {
                chip8->registers.PC += 2;
            }
        break;

        //SE Vx, byte (4xkk): Skip next instruction if Vx != kk
        case 0x4000:
            if (chip8->registers.V[x] != kk)
            {
                chip8->registers.PC += 2;
            }
        break;

        //SE Vx, Vy (5xy0): Skip next instruction if Vx = Vy
        case 0x5000:
            if (chip8->registers.V[x] == chip8->registers.V[y])
            {
                chip8->registers.PC += 2;
            }
        break;

        //LD Vx, byte (6xkk): Puts the value kk into register Vx
        case 0x6000:
            chip8->registers.V[x] == kk;
        break;

        //ADD Vx, byte (7xkk): Adds the value kk to the value of register Vx, then stores the result in Vx.
        case 0x7000:
            chip8->registers.V[x] = chip8->registers.V[x] + kk;
        break;

        case 0x8000:
            chip8_exec_extended_eight (chip8, opcode);
        break;

        // SNE Vx, Vy (9xy0): Skip next instruction if Vx != Vy
        case 0x9000:
            if (chip8->registers.V[x] != chip8->registers.V[y]) {
                chip8->registers.PC += 2;
            }
        break;

        // LD I, addr (Annn): Set I = nnn
        case 0xA000:
            chip8->registers.I = nnn;
        break;

        // JP V0, addr (Bnnn): Jump to location nnn + V0
        case 0xB000:
            chip8->registers.PC = nnn + chip8->registers.V[0];
        break;

        // RND Vx, byte (Cxkk): Set Vx = random byte AND kk
        case 0xC000:
            srand(clock());
            chip8->registers.V[x] = (unsigned char)(rand() & 0xFF) & kk;
        break;

        // DRW Vx, Vy, nibble (Dxyn): Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        case 0xD000:
            const char* sprite = (const char*) &chip8->memory.memory[chip8->registers.I];
            chip8->registers.V[0x0F] = chip8_screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
        break;

        case 0xE000:
            switch (opcode & 0x00FF)
            {
                //SKP Vx (Ex9e): Skip next instruction if key with the value of Vx is pressed.
                case 0x9e:
                    if (chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                    {
                        chip8->registers.PC += 2;
                    }
                break;

                //SKNP Vx (Exa1): Skip next instruction if key with the value of Vx is not pressed.
                case 0xa1:
                 if (!chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]))
                    {
                        chip8->registers.PC += 2;
                    }
                break;
            }
        break;

        case 0xF000:
            chip8_exec_extended_F(chip8, opcode);
        break;

    }
}

void chip8_exec (struct chip8* chip8, unsigned short opcode)
{
    switch (opcode)
    {
        case 0X00E0:
            chip8_screen_clear (&chip8->screen);
        break;

        //Ret: return from a subroutine
        case 0x00EE:
            chip8->registers.PC = chip8_stack_pop (chip8);
        break;

        default:
            chip8_exec_extended (chip8, opcode);

    }
}
