//
//  cpu.h
//  chip8em
//
//  Created by Sebastian Fiorini on 2019-04-26.
//  Copyright © 2019 Sebastian Fiorini. All rights reserved.
//

#ifndef cpu_h
#define cpu_h

#include <stdint.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>

class Cpu;

// Typedef used in for function routines called in assembly table to abstract assembly functionality from CPU memory / architecture
typedef uint16_t (*opRoutine)(Cpu &cpu);

class Cpu {
private:
    const static int CHIP8_MEMORY_SIZE = 4096;
    const static int CHIP8_NUM_REGISTERS = 16;
    
    // Optcode value
    uint16_t opcode;
    
    // Main memory
    // 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
    // 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
    // 0x200-0xFFF - Program ROM and work RAM
    uint8_t memory[CHIP8_MEMORY_SIZE];
    // CPU registers
    uint8_t V[CHIP8_NUM_REGISTERS];
    
    uint16_t I;
    uint16_t pc;
    
    // Display Components
    const static int WIDTH_PIXELS = 64;
    const static int HEIGHT_PIXELS = 32;
    
    uint8_t gfx[WIDTH_PIXELS * HEIGHT_PIXELS];
    bool redraw_required = false;
    
    uint8_t delay_timer;
    uint8_t sound_timer;
    
    // Stack data
    const static int STACK_DEPTH = 16;
    uint16_t stack[STACK_DEPTH];
    uint16_t sp;
    
    // Keypad
    const static int NUM_KEYS = 16;
    uint8_t key[NUM_KEYS];
    
    class Assembly {
    public:
        static uint16_t call_rca1802(Cpu &cpu) {
            // TODO: not really sure what this function does
            return 0;
        }
        static uint16_t disp_clear_screen(Cpu &cpu) {
            memset(cpu.gfx, 0, sizeof(gfx));
            cpu.redraw_required = true;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t flow_return_subroutine(Cpu &cpu) {
            --cpu.sp;
            cpu.pc = cpu.stack[cpu.sp];
            return 0;
        }
        static uint16_t flow_jmp_to_addr(Cpu &cpu) {
            cpu.pc = cpu.opcode & 0x0FFF;
            return 0;
        }
        static uint16_t flow_subroutine(Cpu &cpu) {
            cpu.stack[cpu.sp] = cpu.pc;
            ++cpu.sp;
            cpu.pc = cpu.opcode & 0x0FFF;
            return 0;
        }
        static uint16_t cond_eq_rn(Cpu &cpu) {
            uint8_t r = (cpu.opcode & 0xF0) >> 4;
            uint8_t n = cpu.opcode & 0xFF;
            cpu.pc += 2;
            return cpu.V[r] == n;
        }
        static uint16_t cond_neq_rn(Cpu &cpu) {
            uint8_t r = (cpu.opcode & 0xF0) >> 4;
            uint8_t n = cpu.opcode & 0xFF;
            cpu.pc += 2;
            return cpu.V[r] != n;
        }
        static uint16_t cond_eq_rr(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            cpu.pc += 2;
            return cpu.V[r1] == cpu.V[r2];
        }
        static uint16_t const_set(Cpu &cpu) {
            uint8_t r = (cpu.opcode & 0xF00) >> 8;
            cpu.V[r] = cpu.opcode & 0xFF;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t const_add_eq(Cpu &cpu) {
            uint8_t r = (cpu.opcode & 0xF00) >> 8;
            uint8_t value = cpu.opcode & 0xFF;
            cpu.V[r] = value;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t assign_register(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            cpu.V[r1] = cpu.V[r2];
            cpu.pc += 2;
            return 0;
        }
        static uint16_t bitop_or(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            cpu.V[r1] |= cpu.V[r2];
            cpu.pc += 2;
            return 0;
        }
        static uint16_t bitop_and(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            cpu.V[r1] &= cpu.V[r2];
            cpu.pc += 2;
            return 0;
        }
        static uint16_t bitop_xor(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            cpu.V[r1] ^= cpu.V[r2];
            cpu.pc += 2;
            return 0;
        }
        static uint16_t math_addeq(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            bool set_vf = (cpu.V[r1] + cpu.V[r2] > 0xFF);
            cpu.V[r1] += cpu.V[r2];
            cpu.V[0xF] = set_vf;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t math_subeq(Cpu &cpu) {
            uint8_t r1 = (cpu.opcode & 0xF00) >> 8;
            uint8_t r2 = (cpu.opcode & 0xF0) >> 4;
            // TODO: Check on the comparison operator direction, from the wiki docs I would assume this to be right
            bool set_vf = (cpu.V[r2] < cpu.V[r1]);
            cpu.V[r1] -= cpu.V[r2];
            cpu.V[0xF] = set_vf;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t bitop_rseq(Cpu &cpu) {
            uint8_t r = (cpu.opcode & 0xF00) >> 8;
            uint8_t lsb = cpu.V[r] & 0x1;
            cpu.V[0xF] = lsb;
            cpu.V[r] >>= 1;
            cpu.pc += 2;
            return 0;
        }
        static uint16_t math_sub(Cpu &cpu) {
            return 0;
        }
        static uint16_t bitop_lseq(Cpu &cpu) {
            return 0;
        }
        static uint16_t cond_neq_rr(Cpu &cpu) {
            return 0;
        }
        static uint16_t mem_setaddr(Cpu &cpu) {
            return 0;
        }
        static uint16_t flow_jmp_addr_add(Cpu &cpu) {
            return 0;
        }
        static uint16_t rand_gen(Cpu &cpu) {
            return 0;
        }
        static uint16_t disp_draw_vv(Cpu &cpu) {
            return 0;
        }
        static uint16_t keyop_eq(Cpu &cpu) {
            return 0;
        }
        static uint16_t keyop_neq(Cpu &cpu) {
            return 0;
        }
        static uint16_t timer_set(Cpu &cpu) {
            return 0;
        }
        static uint16_t keyop_get(Cpu &cpu) {
            return 0;
        }
        static uint16_t timer_delay(Cpu &cpu) {
            return 0;
        }
        static uint16_t sound_set(Cpu &cpu) {
            return 0;
        }
        static uint16_t mem_addeq(Cpu &cpu) {
            return 0;
        }
        static uint16_t mem_sprite_add_r(Cpu &cpu) {
            return 0;
        }
        static uint16_t bcd_store(Cpu &cpu) {
            return 0;
        }
        static uint16_t mem_dump(Cpu &cpu) {
            return 0;
        }
        static uint16_t mem_load(Cpu &cpu) {
            return 0;
        }
        
    };
    friend class Assembly;
    
    opRoutine getFuncFromOpcode(uint16_t opcode) {
        uint8_t leading_nibble = opcode >> 12;
        if (leading_nibble == 0) {
            return Assembly::call_rca1802;
        } else if (opcode == 0x00E0) {
            return Assembly::disp_clear_screen;
        } else if (opcode == 0x00EE) {
            return Assembly::flow_return_subroutine;
        } else if (leading_nibble == 1) {
            return Assembly::flow_jmp_to_addr;
        } else if (leading_nibble == 2) {
            return Assembly::flow_subroutine;
        } else if (leading_nibble == 3) {
            return Assembly::cond_eq_rn;
        } else if (leading_nibble == 4) {
            return Assembly::cond_neq_rn;
        } else if (leading_nibble == 5) {
            return Assembly::cond_eq_rr;
        } else if (leading_nibble == 6) {
            return Assembly::const_set;
        } else if (leading_nibble == 7) {
            return Assembly::const_add_eq;
        } else if (leading_nibble == 8) {
            uint8_t endVal = opcode & 0xF;
            switch (endVal) {
                case 0:
                    return Assembly::assign_register;
                case 1:
                    return Assembly::bitop_or;
                case 2:
                    return Assembly::bitop_and;
                case 3:
                    return Assembly::bitop_xor;
                case 4:
                    return Assembly::math_addeq;
                case 5:
                    return Assembly::math_subeq;
                case 6:
                    return Assembly::bitop_rseq;
                case 7:
                    return Assembly::math_sub;
                case 0xE:
                    return Assembly::bitop_lseq;
            }
        } else if (leading_nibble == 9) {
            return Assembly::cond_neq_rr;
        } else if (leading_nibble == 0xA) {
            return Assembly::mem_setaddr;
        } else if (leading_nibble == 0xB) {
            return Assembly::flow_jmp_addr_add;
        } else if (leading_nibble == 0xC) {
            return Assembly::rand_gen;
        } else if (leading_nibble == 0xD) {
            return Assembly::disp_draw_vv;
        } else if (leading_nibble == 0xE) {
            uint8_t last_byte = opcode & 0xFF;
            switch (last_byte) {
                case 0x9E:
                    return Assembly::keyop_eq;
                case 0xA1:
                    return Assembly::keyop_neq;
            }
        } else if (leading_nibble == 0xF) {
            uint8_t last_byte = opcode & 0xFF;
            switch (last_byte) {
                case 0x07:
                    return Assembly::timer_set;
                case 0x0A:
                    return Assembly::keyop_get;
                case 0x15:
                    return Assembly::timer_delay;
                case 0x18:
                    return Assembly::sound_set;
                case 0x1E:
                    return Assembly::mem_addeq;
                case 0x29:
                    return Assembly::mem_sprite_add_r;
                case 0x33:
                    return Assembly::bcd_store;
                case 0x55:
                    return Assembly::mem_dump;
                case 0x65:
                    return Assembly::mem_load;
            }
        }
        return nullptr;
    }
    
public:
    Cpu() : pc{0x200}, opcode{0}, I{0}, sp{0} {
        memset(gfx, 0, WIDTH_PIXELS * HEIGHT_PIXELS * sizeof(gfx[0]));
        memset(stack, 0, STACK_DEPTH * sizeof(stack[0]));
        memset(V, 0, CHIP8_NUM_REGISTERS * sizeof(V[0]));
        memset(memory, 0, CHIP8_MEMORY_SIZE * sizeof(memory[0]));
        
        // Load fonts
        for (int i = 0; i < 80; ++i) {
            // TODO: Fonts?????
            // memory[i] = chip8_fontset[i];
        }
        
        delay_timer = 0;
        sound_timer = 0;
    }
    void load_game(std::string filename) {
        std::ifstream file(filename, std::ios::binary);
        int i = 0x200;
        for (int i = 0x200; i < CHIP8_MEMORY_SIZE && !file.eof(); ++i) {
            uint8_t data;
            file >> data;
            memory[i] = data;
        }
        if (i > CHIP8_MEMORY_SIZE) {
            std::cerr << "Binary file given cannot be loaded in chip8 memory." << std::endl;
        }
    }
    
    void emulate_cycle() {
        // Fetch code
        opcode = memory[pc] << 8 | memory[pc + 1];
        
        // Decode opcode
        // See list of opcode meanings https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
        opRoutine routine = getFuncFromOpcode(opcode);
        if (!routine) {
            std::cerr << "Illegal instruction received: " << std::hex << opcode << std::endl;
            throw;
        }
        
        // Execute opcode
        auto val = routine(*this);
        
        // Update timers, running ideally at 60Hz, calling this function should take 1000/60ms
        // Assume prior code took negligible time
        --delay_timer;
        --sound_timer;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
    }
    
    bool do_redraw() {
        return redraw_required;
    }
    
    void notify_display_redrawn() {
        redraw_required = false;
    }
    
    void set_keys(std::vector<int> keys) {
        
    }
};

#endif /* cpu_h */
