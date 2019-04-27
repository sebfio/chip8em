#include <iostream>
#include <unistd.h>

#include "cpu.h"
#include "gl.h"

int main (int argc, char **argv) {
    if (argc < 2 || argc > 2) {
        std::cout << "Program usage: " << argv[0] << " ROM_GAME_PATH " << std::endl;
        exit(1);
    }
    
    Cpu *cpu = new Cpu();
    
    std::string game_name = argv[1];
    
    //setupGraphics();
    //setupInput();
    
    cpu->load_game(game_name);
    
    bool exitButton = false;
    for (;;) {
        cpu->emulate_cycle();
        
        if (exitButton) {
            break;
        }
        
        if (cpu->do_redraw()) {
            //draw_graphics();
        }
        
        //cpu->set_keys(VECTOR);
    }
    
    delete cpu;
    
    return 0;
}
