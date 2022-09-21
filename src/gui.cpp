#include "gui.hpp"
#include <iostream>

#ifndef _WIN64
    #include <nfd.h>
#endif

GUI::GUI(CPU* cpu) {
    this->cpu = cpu;
}

void GUI::init() {

    if(!HEADLESS) {

        // SDL init
        SDL_Init(SDL_INIT_EVERYTHING);
        IMG_Init(IMG_INIT_PNG);

        // Window, renderer
        window = SDL_CreateWindow("Apple II emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W*scale, SCREEN_H*scale, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

        // Clear renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);

        // Load charset PNG
        SDL_Surface *charset = IMG_Load("roms/charset40.png");
        texCharset = SDL_CreateTextureFromSurface(renderer, charset);
        SDL_FreeSurface(charset);

        // Screen texture
        texScreen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_W, SCREEN_H);
        
        // Detect AZERTY layout
        if(SDL_GetKeyFromScancode(SDL_SCANCODE_Q) == SDLK_a)
            hostAzerty = true;
        
        currentTime = SDL_GetTicks();
        lastTime = currentTime;
    }
    

    // Status
    running = true;

}

void GUI::close() {

    if(HEADLESS)
        return;

    // SDL
    SDL_DestroyTexture(texCharset);
    SDL_DestroyTexture(texScreen);

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GUI::pollEvents() {

    if(HEADLESS)
        return;

    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch(event.type){

            case SDL_QUIT:
                // Close GUI
                close();

                // End program
                running = false;

                break;

            case SDL_KEYDOWN:

                key = event.key.keysym.sym;

                // F2 key resets
                if(key == SDLK_F2) {
                    cpu->reset();
                    break;
                }

                // F3 key loads disk
                if(key == SDLK_F3) {

                    #ifndef _WIN64
                        nfdchar_t *outPath = NULL;
                        nfdresult_t result = NFD_OpenDialog( "dsk,do", NULL, &outPath );

                        if(result == NFD_OKAY) {
                            // Reset Apple 2 with disk
                            if(cpu->mem->disk->loadFile(outPath) == 0)
                                cpu->reset();
                        }
                    #endif

                    break;
                }

                // F11 key changes color modes
                if(key == SDLK_F11) {
                    monochrome = !monochrome;
                    break;
                }

                // Ctrl, alt and shift generate no keypress
                if( key == SDLK_LALT || key == SDLK_RALT ||
                    key == SDLK_LCTRL || key == SDLK_RCTRL ||
                    key == SDLK_LSHIFT || key == SDLK_RSHIFT)
                    break;

                cpu->mem->strobeKeyboardKey(decodeKey(key));

                break;

            case SDL_KEYUP:
                
                // TODO this doesn't account for simultaneous key presses
                cpu->mem->clearKeyboardStrobe();

                break;
        }
    }
}

char GUI::decodeKey(int key) {

    const byte *allKeys = SDL_GetKeyboardState(0);

    keyAlt = allKeys[SDL_SCANCODE_LALT] || allKeys[SDL_SCANCODE_RALT];
    keyShift = allKeys[SDL_SCANCODE_LSHIFT] || allKeys[SDL_SCANCODE_RSHIFT];
    keyCtrl = allKeys[SDL_SCANCODE_LCTRL] || allKeys[SDL_SCANCODE_RCTRL];

    // TODO Refactor. Certain keys not accounted for in swap-azerty mode

    // AZERTY-specific key changes
    if(hostAzerty) {

        switch(key) {
            case SDLK_1 : return (keyShift) ? '1' : '&';
            case SDLK_2 : return (keyShift) ? '2' : '{';
            case SDLK_3 : return (keyShift) ? '3' : '"';
            case SDLK_4 : return (keyShift) ? '4' : '\'';
            case SDLK_5 : return (keyShift) ? '5' : '(';
            case SDLK_6 : return (keyShift) ? '6' : ']';
            case SDLK_7 : return (keyShift) ? '7' : '}';
            case SDLK_8 : return (keyShift) ? '8' : '!';
            case SDLK_9 : return (keyShift) ? '9' : '\\';
            case SDLK_0 : return (keyShift) ? '0' : '@';

            case SDLK_SEMICOLON : return (keyShift) ? '.' : ';';
            case SDLK_COLON :     return (keyShift) ? '/' : ':';
            case SDLK_EXCLAIM :   return (keyShift) ? '+' : '=';
            case SDLK_COMMA :     return (keyShift) ? '?' : ',';
            case 249:             return (keyShift) ? '%' : '|';
            case 1073741824:      return (keyShift) ? '~' : '^';

            case SDLK_LESS :      return (keyShift) ? '>' : '<';
            case SDLK_EQUALS :    return (keyShift) ? '_' : '-';

            case SDLK_DOLLAR :    return (keyShift) ? '*' : '$';
        }
    }


    switch(key) {

        case SDLK_a : return (swapAzerty) ? 'q' : 'a';
        case SDLK_q : return (swapAzerty) ? 'a' : 'q';
        case SDLK_w : return (swapAzerty) ? 'z' : 'w';
        case SDLK_z : return (swapAzerty) ? 'w' : 'z';

        case SDLK_m :         return (swapAzerty) ? ((keyShift) ? ':' : ';') : 'm';
        case SDLK_SEMICOLON : return (swapAzerty) ? 'm' : ((keyShift) ? ':' : ';');

        case SDLK_1 : return (keyShift) ? '!' : '1';
        case SDLK_2 : return (keyShift) ? '"' : '2';
        case SDLK_3 : return (keyShift) ? '@' : '3';
        case SDLK_4 : return (keyShift) ? '$' : '4';
        case SDLK_5 : return (keyShift) ? '%' : '5';
        case SDLK_6 : return (keyShift) ? '^' : '6';
        case SDLK_7 : return (keyShift) ? '&' : '7';
        case SDLK_8 : return (keyShift) ? '*' : '8';
        case SDLK_9 : return (keyShift) ? '(' : '9';
        case SDLK_0 : return (keyShift) ? ')' : '0';
        
        case SDLK_BACKSPACE : return (backspaceFix) ? 8 : 127; // Backspace / Rubout

        case SDLK_LEFT :    return 8;  // Backspace
        case SDLK_RIGHT :   return 21; // Negative acknowledge
        case SDLK_UP :      return 11; // Vertical tabulation
        case SDLK_DOWN :    return 10; // Line feed

        case SDLK_ESCAPE :  return 27; // Escape

        case SDLK_KP_0 :        return '0';
        case SDLK_KP_1 :        return '1';
        case SDLK_KP_2 :        return '2';
        case SDLK_KP_3 :        return '3';
        case SDLK_KP_4 :        return '4';
        case SDLK_KP_5 :        return '5';
        case SDLK_KP_6 :        return '6';
        case SDLK_KP_7 :        return '7';
        case SDLK_KP_8 :        return '8';
        case SDLK_KP_9 :        return '9';
        case SDLK_KP_PLUS :     return '+';
        case SDLK_KP_MINUS :    return '-';
        case SDLK_KP_MULTIPLY : return '*';
        case SDLK_KP_DIVIDE :   return '/';
        case SDLK_KP_ENTER :    return '\r';
        case SDLK_KP_PERIOD :   return '.';

        default : return key;
    }
}

void GUI::update() {

    if(HEADLESS)
        return;


    // Text mode addresses in RAM
    const int row[24]= {0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 
        0x0780, 0x0428, 0x04A8, 0x0528, 0x05A8, 0x0628, 0x06A8, 0x0728, 0x07A8, 
        0x0450, 0x04D0, 0x0550, 0x05D0, 0x0650, 0x06D0, 0x0750, 0x07D0};
    
    
    // SDL
    SDL_SetRenderTarget(renderer, texScreen);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Text mode
    if(cpu->mem->sw_text || cpu->mem->sw_mixed) {

        byte firstRow = (cpu->mem->sw_text) ? 0 : 20;

        for(byte y = firstRow; y < 24 ; y++) {
            for(byte x = 0; x < 40 ; x++) {

                int addr = row[y] + x;

                // Page 2
                if(cpu->mem->sw_page2)
                    addr += 0x400;

                byte character = cpu->mem->readByte(addr);
                byte charset = character / 64;

                switch(charset) {
                    case 1: charset = (flashTimer < flashDuration / 2) ? 2 : 0; break; // Flashing cursor
                    case 3: charset = 2; break;                                        // Extra charset disabled on original Apple II
                }

                byte displayChar = character % 64;
                
                // Charset texture rect
                SDL_Rect rectSrc = {
                    (displayChar % 16) * 8,
                    (displayChar / 16) * 8 + charset * 32,
                    7, 8
                };

                // Destination rect
                SDL_Rect rectDst = {x*7, y*8, 7, 8};
                
                SDL_RenderCopy(renderer, texCharset, &rectSrc, &rectDst);
            }
        }
    }

    // Low-resolution graphics
    if(!cpu->mem->sw_text && !cpu->mem->sw_hires) {
        
        SDL_Color loResColors[] = {
            {  0,   0,   0}, //  0 Black
            {227,  30,  96}, //  1 Red
            { 96,  78, 189}, //  2 Dark blue
            {255,  68, 253}, //  3 Purple
            {  0, 163,  96}, //  4 Dark green
            {156, 156, 156}, //  5 Grey
            { 20, 207, 253}, //  6 Med blue
            {208, 195, 255}, //  7 Lt blue
            { 96, 114,   3}, //  8 Brown
            {255, 106,  60}, //  9 Orange
            {156, 156, 156}, // 10 Grey
            {255, 160, 208}, // 11 Pink
            { 20, 245,  60}, // 12 Lt green
            {208, 221, 141}, // 13 Yellow
            {114, 255, 208}, // 14 Aqua
            {255, 255, 255}  // 15 White
        };

        byte lastRow = (cpu->mem->sw_mixed) ? 20 : 24;

        for(byte y = 0; y < lastRow ; y++) {
            for(byte x = 0; x < 40 ; x++) {

                int addr = row[y] + x;

                // Page 2
                if(cpu->mem->sw_page2)
                    addr += 0x400;
                    
                // Read color byte
                byte byteCol = cpu->mem->readByte(addr);

                // Read both nibbles
                for(byte k = 0 ; k < 2 ; k++) {

                    byte nibble = byteCol;
                    
                    nibble &= (0x0f << (k*4));
                    nibble >>= k*4;
                    
                    SDL_Color col = loResColors[nibble];

                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
                    
                    SDL_Rect plotRect = {
                        x*7, y*8 + 4*k,
                        7, 4
                    };

                    SDL_RenderFillRect(renderer, &plotRect);
                }
                    
            }
        }
    }
    
    // High-resolution graphics
    if(cpu->mem->sw_hires && !cpu->mem->sw_text) {

        int addr = (cpu->mem->sw_page2) ? 0x4000 : 0x2000;

        SDL_Color hiResColor[][4] = {
            {
                {  0,   0,   0}, // 0 - Black
                { 20, 245,  60}, // 1 - Green
                {255,  68, 253}, // 2 - Purple
                {255, 255, 255}  // 3 - White
            },
            {
                {  0,   0,   0}, // 0 - Black
                {255, 106,  60}, // 1 - Orange
                { 20, 207, 253}, // 2 - Blue
                {255, 255, 255}  // 3 - White
            }
        };

        word boxAddr[24] = {
            0x0000, 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380,
            0x0028, 0x00a8, 0x0128, 0x01a8, 0x0228, 0x02a8, 0x0328, 0x03a8,
            0x0050, 0x00d0, 0x0150, 0x01d0, 0x0250, 0x02d0, 0x0350, 0x03d0
        };

        byte lastLine = (cpu->mem->sw_mixed) ? 160 : 192;

        for(byte line = 0; line < lastLine ; line++) {
            
            // Interleaving pattern
            word lineMemOffset = boxAddr[line >> 3] + ((line & 0x7) * 0x0400);
            
            // Monochrome pixel data buffer
            byte rawLine[35] = {0};

            // Palette bit for each 7-pixel group
            byte linePal[40];

            // Pixel location in buffer
            byte index = 0;
            byte bit = 0;

            // Copy pixel data in buffer
            for(byte blockOffset = 0 ; blockOffset < 40 ; blockOffset ++) {

                byte lineData = cpu->mem->readByte(addr + lineMemOffset + blockOffset);

                // Copy first 7 bits
                for(byte dataBit = 0; dataBit < 7 ; dataBit++) {
                    byte pixel = (lineData >> dataBit) & 0x1;
                    rawLine[index] |= pixel << (7 - bit);

                    // Move to next byte
                    bit ++;
                    if(bit > 7) {
                        index ++;
                        bit = 0;
                    }
                }

                // Copy palette bit
                linePal[blockOffset] = lineData >> 7;
            }


            // Draw dots
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            SDL_Color color;

            // Color palette
            byte value = 0;
            byte palette = 0;

            // Previous pixel
            byte prevPixel = 0;

            // 8-dot columns
            for(byte col = 0; col < 35 ; col ++) {

                // 7-dot block index
                byte blockOffset = 0;
                byte blockIndex = 0;
                
                // 8 pixels
                for(byte pixel = 0 ; pixel < 8 ; pixel ++) {

                    // Current pixel
                    byte currPixel = (rawLine[col] & (0x80 >> (pixel)));

                    byte nextPixel = 0;

                    // Check next pixel
                    if(pixel != 7 || col != 34) {
                        byte nextCol = (pixel == 7) ? col+1 : col;

                        if(rawLine[nextCol] & (0x80 >> ((pixel+1) & 0x7))) {
                            nextPixel = 1;
                        }
                            
                    } else
                        nextPixel = prevPixel;
                    
                    if(!monochrome) {
                        // Set dot color   
                        if(!(pixel & 0x1)) {
                            // Read two adjacent graphics bits
                            value = (rawLine[col] >> (6 - pixel)) & 0x3;

                            // Palette bit
                            palette = linePal[blockIndex];
                        }                            
                        
                        if(currPixel && (nextPixel || prevPixel)) {
                            // Adjacent pixels become white
                            value = 3;
                        }
                            
                        // RGB values
                        color = hiResColor[palette][value];

                        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
                    }
                    
                    
                    
                    // Draw dot
                    if(currPixel || (!monochrome && nextPixel && prevPixel && (value - 1) < 2)) {

                        SDL_Rect pixelRect = {
                            col * 8 + pixel,
                            line, 
                            1, 1
                        };

                        SDL_RenderFillRect(renderer, &pixelRect);
                    }

                    // 7-dot block index
                    blockOffset ++;
                    if(blockOffset > 6) {
                        blockIndex++;
                        blockOffset = 0;
                    }

                    prevPixel = currPixel;

                }
            }
        }


    }
    

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texScreen, 0, 0);
    SDL_RenderPresent(renderer);

    // Cursor flash
    flashTimer -= 1;
    
    if(flashTimer < 0)
        flashTimer += flashDuration;
}

void GUI::delay() {

    if(HEADLESS)
        return;

    //60fps delay
    currentTime = SDL_GetTicks();

    if(currentTime - lastTime < 1000/60) {
        SDL_Delay(1000/60 - (currentTime - lastTime));
    }

    lastTime = currentTime;

}