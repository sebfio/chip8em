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

class Cpu;

// Typedef used in for function routines called in assembly table to abstract assembly functionality from CPU memory / architecture
typedef void * (*opRoutine)(Cpu &cpu, uint16_t);

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
        static void * call_rca1802(Cpu &cpu, uint16_t opcode) {
            // TODO: not really sure what this function does			
            return nullptr;
        }
        static void * disp_clear_screen(Cpu &cpu, uint16_t opcode) {
            memset(cpu.gfx, 0, sizeof(gfx));
            cpu.redraw_required = true;
            return nullptr;
        }
        static void * flow_return_subroutine(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * flow_jmp_to_addr(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * flow_subroutine(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * cond_eq_rn(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * cond_neq_rn(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * cond_eq_rr(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * const_set(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * const_add(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * assign_register(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bitop_or(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bitop_and(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bitop_xor(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * math_addeq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * math_subeq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bitop_rseq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * math_sub(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bitop_lseq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * cond_neq_rr(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * mem_setaddr(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * flow_jmp_addr_add(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * rand_gen(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * disp_draw_vv(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * keyop_eq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * keyop_neq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * timer_set(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * keyop_get(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * timer_delay(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * sound_set(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * mem_addeq(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * mem_sprite_add_r(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * bcd_store(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * mem_dump(Cpu &cpu, uint16_t opcode) {
            return nullptr;
        }
        static void * mem_load(Cpu &cpu, uint16_t opcode) {
            return nullptr;
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
            return Assembly::const_add;
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
        } else {
            return nullptr;
        }
        return nullptr;
    }
    
public:
    void load_game(std::string filename) {
        
    }
    
    void emulate_cycle() {
        // Fetch code
        uint16_t opcode;
        opcode = memory[pc] << 8 | memory[pc + 1];
        
        // Decode opcode
        // See list of opcode meanings https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
        opRoutine routine = getFuncFromOpcode(opcode);
        if (!routine) {
            std::cerr << "Illegal instruction received: " << std::hex << opcode << std::endl;
            throw;
        }
        // Execute opcode
        uint16_t data = opcode;
        auto val = routine(*this, data);
        
        // Update pc
        pc += 2;
        
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
