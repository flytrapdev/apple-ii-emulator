#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "cpu.hpp"
#include "mem.hpp"
#include "gui.hpp"
#include "test.hpp"
#include "testmem.hpp"

#include "nfd.h"

#ifdef _WIN64
#include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow) {
    
    return main(__argc, __argv);
}
#endif

int main(int argc, char *argv[]) {

    std::cout << "Apple 2 emulator" << std::endl;
    
    bool enableTests = false;

    bool step = false;

    // Emulator components
    Mem* mem = new Mem();
    CPU* cpu = new CPU(mem);
    GUI* gui = new GUI(cpu);

    // CPU tests
    if(enableTests) {
        TestSuite *testsuite = new TestSuite(new CPU(new TestMem()));
        testsuite->run();
    }

    // Init emulation
    cpu->reset();
    gui->init();

    // Read file from command line argument
    if(argc > 1) {
        cpu->mem->disk->diskImage->loadFile(argv[1]);
    }

    std::string dummy;

    while(gui->running) {
        
        
        //for(int i = 0; i < 20 ; i++) {
        //    cpu->emulateInstruction();
        //    cpu->printOpcode(cpu->pc);
        //    cpu->printRegisters();
        //}

        if(HEADLESS) {

            // Headless mode with step by step instructions

            step = true;

            if(step) {
                //getline(std::cin, dummy);
                cpu->printOpcode(cpu->pc);
                cpu->printRegisters();

                std::cin.clear();
                std::cin.sync();

            }

            cpu->emulateInstruction();

            
        }
        else {

            // Normal emulation
            cpu->emulateCycles(17050);
            // TURBO MODE ENGAGED
            //cpu->emulateCycles(999999);
        
            gui->update();
            gui->pollEvents();
            gui->delay();
        }
        
        
        
        //gui->running = false;
    }
    

    return 0;
}