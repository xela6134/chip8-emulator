#ifndef CHIP8_CHIP8EMULATOR_H
#define CHIP8_CHIP8EMULATOR_H

#include <cstdint>
#include <string>

/**
 * Mimics a CPU, Memory, Display, and Input (Keypad) to run CHIP8 programs.
 * - Opcode Processing: Reads 2-byte instructions (aka Opcodes) from memory.
 * - Memory: A 4k (4096-byte) RAM, has reserved areas for system and program data.
 * - Registers: 
 *     -  V) 16 general-purpose 8-bit registers
 *     -  I) 16-bit index register
 *     - pc) 16-bit program counter, initialised as 0x0200 because
 *           0x0200 as 0x0000 ~ 0x1FF is saved for internal program data.
 *           0x0000 ~ 0x01FF is reserved for system use (for interpreter)
 *     - sp)  8-bit stack pointer
 * - stack[16]: 16-level stack, which is able to store 16 16-bit values.
 * - delay_timer, sound_timer: Both 8-bit
 * - Display: 64 x 32 display screen
 * - Keypad: Hexadecimal keypad (1 ~ F)
 * - draw_flag: Represents whether or not display should be re-drawn or not after executing an opcode.
 * - get_nibble: Helper function which extracts a specific set of 4 bits (aka nibble) from an int value.
 */
class Chip8 {
public:
    Chip8();
    bool load_rom(std::string);
    bool get_draw_flag();
    void set_draw_flag(bool);
    void single_cycle();
    int get_display_value(int);
    void set_keypad_value(int, int);
    ~Chip8();
private:
    // CPU
    uint8_t V[16];
    uint16_t I = 0;
    uint16_t pc = 0x0200;
    uint8_t sp = 0;
    uint16_t stack[16];
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    uint8_t memory[4096];
    int display[64 * 32];
    int keypad[16];
    bool draw_flag = false;

    int get_nibble(int, int, int);
};

#endif //CHIP8_CHIP8EMULATOR_H
