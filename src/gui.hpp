#ifndef GUI_HPP
#define GUI_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include "cpu.hpp"

#define SCREEN_W 280
#define SCREEN_H 192

#define HEADLESS 0

struct GUI {

    GUI(CPU* cpu);

    CPU* cpu;

    // Keyboard settings
    uint32 key;

    bool swapAzerty = false;
    bool backspaceFix = true;

    bool hostAzerty = false;
    
    bool keyShift = false;
    bool keyCtrl = false;
    bool keyAlt = false;

    // Settings
    bool enableNCURSES = false;
    bool monochrome = false;

    // SDL
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *texCharset;
    SDL_Texture *texScreen;

    // Display scale
    uint32 scale = 3;

    // Timing
    uint32 lastTime, currentTime;
    float remainderTime;

    // Flashing cursor
    int flashDuration = 60;
    int flashTimer;

    bool running;

    char decodeKey(int key);

    void init();
    void pollEvents();
    void update();
    void delay();

    void close();

};

#endif