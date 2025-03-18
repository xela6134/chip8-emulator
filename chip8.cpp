#include <cstring>
#include <fstream>
#include <iostream>
#include "chip8.h"

// Constructor
Chip8::Chip8() {
    // Clear Registers, Stack and Memory
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));

    // Resetting display and keypad
    memset(display, 0, sizeof(display));
    memset(keypad, 0, sizeof(keypad));

    // No built-in character rendering for CHIP-8, we need to specify this to show the display.
    // Loaded in the first 80 bytes of memory.
    unsigned char chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Load fontset from 0 to 80
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }
}

// Loads ROM, with path to ROM given as argument
bool Chip8::load_rom(std::string rom_path) {
    std::ifstream f(rom_path, std::ios::binary | std::ios::in);
    if (!f.is_open()) {
        return false;
    }

    // Load in memory from 0x200 (512) onwards
    char c;
    int j = 512;
    for (int i = 0x0200; f.get(c); ++i) {
        memory[i] = (uint8_t)c;
        j++;
    }
    return true;
}

// Getters and Setters
bool Chip8::get_draw_flag() {
    return draw_flag;
}

void Chip8::set_draw_flag(bool flag) {
    draw_flag = flag;
}

int Chip8::get_display_value(int i) {
    return display[i];
}

void Chip8::set_keypad_value(int index, int val) {
    keypad[index] = val;
}

// Emulates one cycle
void Chip8::single_cycle() {
    // Combine two bytes into a 2-byte opcode.
    // E.g., if memory[pc] = 0x12 and memory[pc+1] = 0x34, then
    // opcode = (0x12 << 8) | 0x34 = 0x1234.
    int opcode = (memory[pc] << 8) | (memory[pc + 1]);

    int opcode_msb_nibble = get_nibble(opcode, 12, 0xF000);
    int val, reg, reg1, reg2;

    switch (opcode_msb_nibble) {
        // Opcodes starting with 0.
        // 0x00E0: Clears the entire display.
        // 0x00EE: Returns control after a subroutine call (pops the return address from stack).
        case 0:
            switch (opcode) {
                case 0x00E0:
                    memset(display, 0, sizeof(display));
                    draw_flag = true;
                    pc += 2;
                    break;
                case 0x00EE:
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                default:
                    std::cerr << "Invalid opcode: " << std::hex << opcode << std::endl;
                    break;
            }
            break;
        // Opcode 1NNN: Jump to address NNN.
        case 1:
            pc = opcode & 0x0FFF;
            break;
        // Opcode 2NNN: Call subroutine at NNN.
        //          Push current pc onto the stack, then set pc to NNN.
        case 2:
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;
        // Opcode 3XNN: Skip next instruction if V[X] == NN.
        case 3:
            val = get_nibble(opcode, 0, 0x00FF); // Extract NN.
            reg = get_nibble(opcode, 8, 0x0F00); // Extract register index X.
            pc += 2;
            if (V[reg] == val) {
                pc += 2;
            }
            break;
        // Opcode 4XNN: Skip next instruction if V[X] != NN.
        case 4:
            val = get_nibble(opcode, 0, 0x00FF);
            reg = get_nibble(opcode, 8, 0x0F00);
            pc += 2;
            if (V[reg] != val) {
                pc += 2;
            }
            break;
        // Opcode 5XY0: Skip next instruction if V[X] == V[Y].
        case 5:
            reg1 = get_nibble(opcode, 8, 0x0F00);
            reg2 = get_nibble(opcode, 4, 0x00F0);
            pc += 2;
            if (V[reg1] == V[reg2]) {
                pc += 2;
            }
            break;
        // Opcode 6XNN: Sets V[X] to NN.
        case 6:
            val = get_nibble(opcode, 0, 0x00FF);
            reg = get_nibble(opcode, 8, 0x0F00);
            V[reg] = val;
            pc += 2;
            break;
        // Opcode 7XNN: Adds NN to V[X].
        case 7:
            val = get_nibble(opcode, 0, 0x00FF);
            reg = get_nibble(opcode, 8, 0x0F00);
            V[reg] += val;
            pc += 2;
            break;
        // Arithmetic and Bitwise Operations (Opcodes starting with 8).
        case 8:
            val = get_nibble(opcode, 0, 0x000F);
            switch (val) {
                case 0:
                    // 8XY0: Sets V[X] = V[Y].
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    V[reg1] = V[reg2];
                    pc += 2;
                    break;
                case 1:
                    // 8XY1: Sets V[X] = V[X] OR V[Y].
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    V[reg1] |= V[reg2];
                    V[0xF] = 0;
                    pc += 2;
                    break;
                case 2:
                    // 8XY2: Sets V[X] = V[X] AND V[Y].
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    V[reg1] &= V[reg2];
                    V[0xF] = 0;
                    pc += 2;
                    break;
                case 3:
                    // 8XY3: Sets V[X] = V[X] XOR V[Y].
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    V[reg1] ^= V[reg2];
                    V[0xF] = 0;
                    pc += 2;
                    break;
                case 4:
                    // 8XY4: Adds V[Y] to V[X]. Sets V[F] to 1 if there is a carry.
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    if (V[reg1] + V[reg2] > 0xFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[reg1] += V[reg2];
                    V[reg1] = static_cast<int8_t>(V[reg1]);
                    pc += 2;
                    break;
                case 5:
                    // 8XY5: Subtracts V[Y] from V[X]. Sets V[F] to 0 if there is a borrow.
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    if (V[reg1] < V[reg2]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[reg1] = static_cast<uint8_t>(V[reg1]) - static_cast<uint8_t>(V[reg2]);
                    pc += 2;
                    break;
                case 6:
                    // 8XY6: Shifts V[X] right by one. Stores the least significant bit of V[X] in V[F].
                    reg = get_nibble(opcode, 8, 0x0F00);
                    V[0xF] = V[reg] & 0x1;
                    V[reg] >>= 1;
                    V[reg] = static_cast<uint8_t>(V[reg]);
                    pc += 2;
                    break;
                case 7:
                    // 8XY7: Sets V[X] = V[Y] - V[X]. Sets V[F] to 0 if there is a borrow.
                    reg1 = get_nibble(opcode, 8, 0x0F00);
                    reg2 = get_nibble(opcode, 4, 0x00F0);
                    if (V[reg1] > V[reg2]) {
                        V[0xF] = 0;
                    } else {
                        V[0xF] = 1;
                    }
                    V[reg1] = static_cast<uint8_t>(V[reg2]) - static_cast<uint8_t>(V[reg1]);
                    pc += 2;
                    break;
                case 0xE:
                    // 8XYE: Shifts V[X] left by one. Stores the most significant bit of V[X] in V[F].
                    reg = get_nibble(opcode, 8, 0x0F00);
                    V[0xF] = V[reg] >> 7;
                    V[0xF] = static_cast<uint8_t>(V[0xF]);
                    V[reg] <<= 1;
                    V[reg] = static_cast<uint8_t>(V[reg]);
                    pc += 2;
                    break;
                default:
                    std::cerr << "Invalid opcode: " << std::hex << opcode << std::endl;
                    break;
            }
            break;
        // Opcode 9XY0: Skip next instruction if V[X] != V[Y].
        case 9:
            reg1 = get_nibble(opcode, 8, 0x0F00);
            reg2 = get_nibble(opcode, 4, 0x00F0);
            pc += 2;
            if (V[reg1] != V[reg2]) {
                pc += 2;
            }
            break;
        // Opcode ANNN: Sets I to the address NNN.
        case 10:
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        // Opcode BNNN: Jumps to the address computed by adding NNN to V[0].
        case 11:
            pc = (opcode & 0x0FFF);
            pc += V[0];
            break;
        // Opcode CXNN: Generates a random number, ANDs it with NN, and stores the result in V[X].
        case 12: {
            val = get_nibble(opcode, 0, 0x00FF);
            reg = get_nibble(opcode, 8, 0x0F00);
            int random_number = rand() % 256;
            V[reg] = random_number & val;
            pc += 2;
            break;
        }
        // Opcode DXYN: Draws a sprite at coordinates (V[X], V[Y]) with a height of N pixels.
        case 13: {
            reg1 = get_nibble(opcode, 8, 0x0F00);
            reg2 = get_nibble(opcode, 4, 0x00F0);
            int height = opcode & 0x000F;
            int width = 8;
            V[0xF] = 0;

            // X & Y coordinates.
            int x = V[reg1];
            int y = V[reg2];

            for (int i = 0; i < height; i++) {
                int pixel = memory[I + i];
                for (int j = 0; j < width; j++) {
                    if ((pixel & (0x80 >> j)) != 0) {
                        int index = ((x + j) + ((y + i) * 64)) % 2048;
                        if (display[index] == 1) {
                            V[0xF] = 1;
                        }
                        display[index] ^= 1;
                    }
                }
            }

            draw_flag = true;
            pc += 2;
            break;
        }
        // Opcodes starting with E (Keypad operations).
        // EX9E: Skip next instruction if key in V[X] is pressed.
        // EXA1: Skip next instruction if key in V[X] isn't pressed.
        case 14:
            val = get_nibble(opcode, 0, 0x00FF);
            switch (val) {
                case 0x9E:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    pc += 2;
                    if (keypad[V[reg]] != 0) {
                        pc += 2;
                    }
                    break;
                case 0xA1:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    pc += 2;
                    if (keypad[V[reg]] == 0) {
                        pc += 2;
                    }
                    break;
                default:
                    std::cerr << "Invalid opcode: " << std::hex << opcode;
                    break;
            }
            break;
        // Opcodes starting with F (Timer and Memory operations).
        // FX07: Sets V[X] to the value of the delay timer.
        // FX0A: Awaits a key press and stores it in V[X].
        // FX15: Sets the delay timer to V[X].
        // FX1E: Adds V[X] to I.
        // FX18: Increment to next instruction.
        // FX29: Sets I to the sprite location for the character in V[X].
        // FX33: Stores the BCD representation of V[X] in memory.
        // FX55: Stores registers V0 through V[X] in memory starting at I.
        // FX65: Fills registers V0 through V[X] with values from memory starting at I.
        case 15: {
            val = get_nibble(opcode, 0, 0x00FF);
            switch (val) {
                case 0x07:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    V[reg] = delay_timer;
                    pc += 2;
                    break;
                case 0x0A: {
                    bool key_pressed = false;
                    reg = get_nibble(opcode, 8, 0x0F00);
                    for (int i = 0; i < 16; i++) {
                        if (keypad[i] != 0) {
                            key_pressed = true;
                            V[reg] = (uint8_t)i;
                        }
                    }
                    if (key_pressed) {
                        pc += 2;
                    }
                    break;
                }
                case 0x15:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    delay_timer = V[reg];
                    pc += 2;
                    break;
                case 0x18:
                    pc += 2;
                    break;
                case 0x1E:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    if (I + V[reg] > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[reg];
                    I = (uint16_t)I;
                    pc += 2;
                    break;
                case 0x29:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    I = V[reg] * 0x5;
                    pc += 2;
                    break;
                case 0x33:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    memory[I] = (uint8_t)(V[reg] / 100);
                    memory[I + 1] = (uint8_t)((V[reg] / 10) % 10);
                    memory[I + 2] = (uint8_t)(V[reg] % 10);
                    pc += 2;
                    break;
                case 0x55:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    for (int i = 0; i <= reg; i++) {
                        memory[I + i] = V[i];
                    }
                    I = I + reg + 1;
                    I = (uint16_t)I;
                    pc += 2;
                    break;
                case 0x65:
                    reg = get_nibble(opcode, 8, 0x0F00);
                    for (int i = 0; i <= reg; i++) {
                        V[i] = memory[I + i];
                    }
                    I = I + reg + 1;
                    I = (uint16_t)I;
                    pc += 2;
                    break;
                default:
                    std::cerr << "Invalid opcode: " << std::hex << opcode << std::endl;
                    break;
            }
            break;
        }
        default:
            std::cerr << "Invalid opcode: " << std::hex << opcode << std::endl;
            break;
    }

    if (delay_timer > 0) {
        delay_timer--;
    }
}

// Destructor
Chip8::~Chip8() {}

// Extracts 4 bits from opcode.
// e.g., opcode = 0xABCD, bitmask = 0xF000, shift = 12:
// (0xABCD & 0xF000) >> 12 = (0xA000) >> 12 = 0x000A
int Chip8::get_nibble(int opcode, int shift, int bitmask) {
    return ((opcode & bitmask) >> shift);
}
