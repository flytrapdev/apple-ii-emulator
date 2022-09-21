#include "test.hpp"
#include <assert.h>
#include <iostream>

TestSuite::TestSuite(CPU* cpu) {
    this->cpu = cpu;
}

int TestSuite::run() {
    
    int return_code = cpu->loadFile("/home/matthieu/Code/apple2emu/roms/6502_functional_test.bin", 0, false);

    if(return_code != 0) {
        std::cout << "Error reading test rom file" << std::endl;
        return 1;
    }

    cpu->reset();

    // Force program start at $0400
    cpu->pc = 0x400;
    
    bool running = true;

    word prevPC = 0x400;

    while(running) {

        cpu->emulateInstruction();

        if(cpu->pc == 0x3469) {
            std::cout << "Tests OK" << std::endl;
            running = false;
        }

        if(cpu->pc == prevPC) {
            std::cout << "Tests stuck on $" << std::hex << (int)cpu->pc << std::endl;
            return 1;
        }

        prevPC = cpu->pc;
    }

    return 0;
}