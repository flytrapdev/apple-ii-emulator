#include <iostream>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

#include "mem.hpp"
#include "disk_drive.hpp"

using byte = unsigned char;
using word = unsigned short;
using uint32 = unsigned int;

Mem::Mem() {
    disk = new Disk();
}

// Clear memory and load peripheral ROMs
void Mem::init() {

    // Clear RAM
    for(uint32 i = 0 ; i < MAX_SIZE ; i++) {
        data[i] = 0;
    }

    // Load Apple II ROM
    loadFile("roms/apple.rom", 0xd000, false);

    // Copy Disk II ROM if a disk is present
    if(disk->diskImage->loaded) {
        for(int i = 0 ; i < 256 ; i++)
            data[0xc600 + i] = disk -> bootstrapROM[i];
    }
        
}

// Read byte from memory
// handles IO and soft switches
byte Mem::doRead(uint32 addr) {

    word firstByte = (addr & 0xff00);

    // Zero page
    if(firstByte == 0x0000) {
        if(sw_altzp)
            return auxData[addr];

        return data[addr];
    }

    // Soft switches
    else if(firstByte == 0xc000) {
        switch(addr) {

            case 0xc000: return keyboardKey;    // R Read keyboard data
            
            case 0xc010: {                      // R7 Read keyboard flag status
                byte strobe = keyboardKey & 0x80;
                clearKeyboardStrobe();

                return strobe;
            }

            case 0xc013: return sw_ramrd;
            case 0xc014: return sw_ramwrt;
            case 0xc015: return sw_intcxrom;
            case 0xc016: return sw_altzp;
            case 0xc017: return sw_slotc3rom;
            case 0xc018: return sw_80store;
            case 0xc019: return 0; // TODO : return VBLANK
            case 0xc01a: return sw_text;
            case 0xc01b: return sw_mixed;
            case 0xc01c: return sw_page2;
            case 0xc01d: return sw_hires;
            case 0xc01e: return sw_altcharset;
            case 0xc01f: return sw_80col;
            
            case 0xc020: 
                // R Toggle cassette output port
                break;
            
            case 0xc030: 
                // R Toggle speaker
                break;
            
            case 0xc040: 
                // R Generate game controller IO strobe signal
                break;
            

            case 0xc050: sw_text = 0;       break;
            case 0xc051: sw_text = 1;       break;
            case 0xc052: sw_mixed = 0;      break;
            case 0xc053: sw_mixed = 1;      break;
            case 0xc054: sw_page2 = 0;      break;
            case 0xc055: sw_page2 = 1;      break;
            case 0xc056: sw_hires = 0;      break;
            case 0xc057: sw_hires = 1;      break;

            case 0xc058: sw_an0 = 0;        break;
            case 0xc059: sw_an0 = 1;        break;
            case 0xc05a: sw_an1 = 0;        break;
            case 0xc05b: sw_an1 = 1;        break;
            case 0xc05c: sw_an2 = 0;        break;
            case 0xc05d: sw_an2 = 1;        break;
            case 0xc05e: sw_an3 = 0;        break;
            case 0xc05f: sw_an3 = 1;        break;

            case 0xc060: 
                // R7 Check cassette input
                break;
            
            case 0xc061: 
                // R7 push button 0
                break;
            
            // Disk
            case 0xc0e0: 
            case 0xc0e1: 
            case 0xc0e2: 
            case 0xc0e3: 
            case 0xc0e4: 
            case 0xc0e5: 
            case 0xc0e6: 
            case 0xc0e7: 
            case 0xc0e8: 
            case 0xc0e9: 
            case 0xc0ea: 
            case 0xc0eb: 
            case 0xc0ec: 
            case 0xc0ed: 
            case 0xc0ee: 
            case 0xc0ef: 
                return disk->diskRead(addr);
                break;
            
            default: return data[addr];
        }
    }
    else {
        return data[addr];
    }
    

    return 0;
}

// Write byte to memory
// handles IO and soft switches
void Mem::doWrite(uint32 addr, byte value) {

    if(addr >= 0xd000)
        return;

    switch(addr) {

        case 0xc000: sw_80store = 0;    break;
        case 0xc001: sw_80store = 1;    break;
        case 0xc002: sw_ramrd = 0;      break;
        case 0xc003: sw_ramrd = 1;      break;
        case 0xc004: sw_ramwrt = 0;     break;
        case 0xc005: sw_ramwrt = 1;     break;
        case 0xc006: sw_intcxrom = 0;   break;
        case 0xc007: sw_intcxrom = 1;   break;
        case 0xc008: sw_altzp = 0;      break;
        case 0xc009: sw_altzp = 1;      break;
        case 0xc00a: sw_slotc3rom = 0;  break;
        case 0xc00b: sw_slotc3rom = 1;  break;
        case 0xc00c: sw_80col = 0;      break;
        case 0xc00d: sw_80col = 1;      break;
        case 0xc00e: sw_altcharset = 0; break;
        case 0xc00f: sw_altcharset = 1; break;

        case 0xc050: sw_text = 0;       break;
        case 0xc051: sw_text = 1;       break;
        case 0xc052: sw_mixed = 0;      break;
        case 0xc053: sw_mixed = 1;      break;
        case 0xc054: sw_page2 = 0;      break;
        case 0xc055: sw_page2 = 1;      break;
        case 0xc056: sw_hires = 0;      break;
        case 0xc057: sw_hires = 1;      break;

        case 0xc058: sw_an0 = 0;        break;
        case 0xc059: sw_an0 = 1;        break;
        case 0xc05a: sw_an1 = 0;        break;
        case 0xc05b: sw_an1 = 1;        break;
        case 0xc05c: sw_an2 = 0;        break;
        case 0xc05d: sw_an2 = 1;        break;
        case 0xc05e: sw_an3 = 0;        break;
        case 0xc05f: sw_an3 = 1;        break;


        case 0xc080: 
        case 0xc081: 
        case 0xc082: 
        case 0xc083: 
        case 0xc084: 
        case 0xc085: 
        case 0xc086: 
        case 0xc087: 
        case 0xc088: 
        case 0xc089: 
        case 0xc08a: 
        case 0xc08b: 
        case 0xc08c: 
        case 0xc08d: 
        case 0xc08e: 
        case 0xc08f: 
            disk->diskWrite(addr, value);
            break;


        default: data[addr] = value; break;
    }
}

// Read byte wrapper
byte Mem::readByte(uint32 addr) {
    if(addr < MAX_SIZE)
        return doRead(addr);
    
    return 0;
}

// Read word wrapper
word Mem::readWord(uint32 addr) {
    if(addr < MAX_SIZE - 1)
        return (doRead(addr + 1) << 8) | doRead(addr);

    return 0;
}

// Write byte wrapper
void Mem::writeByte(uint32 addr, byte b) {
    if(addr < MAX_SIZE)
        doWrite(addr, b);
}

// Write word wrapper
void Mem::writeWord(uint32 addr, word w) {
    if(addr < MAX_SIZE - 1) {
        doWrite(addr, w & 0x00ff);
        doWrite(addr + 1, (w & 0xff00) >> 8);
    }
}

byte* Mem::getAddr(uint32 addr) {
    if(addr >= MAX_SIZE)
        return data;

    return data + addr;
}

// Clear keyboard strobe
void Mem::clearKeyboardStrobe() {
    keyboardKey &= 0x7f;
}

// Set keyboard key
void Mem::strobeKeyboardKey(byte ascii) {
    keyboardKey = ascii | 0x80; // Set keyboard strobe to 1
}

// Load binary file into memory
int Mem::loadFile(std::string filename, word addr, bool aux) {

    char buffer[MAX_SIZE];

    std::ifstream in;

    in.open(filename, std::ios::in | std::ios::binary);

    if(!in.is_open()) {
        std::cout << "Could not read ROM file " << filename << std::endl;
        return 1;
    }

    in.read(buffer, MAX_SIZE);

    uint32 size = in.gcount();

    std::cout << "ROM size is " << std::dec << size << " bytes" << std::endl;

    if(aux) {
        for(uint32 i = 0; i < size && i + addr < MAX_SIZE; i++) {
            auxData[addr + i] = buffer[i];
        }
    }
    else {
        for(uint32 i = 0; i < size && i + addr < MAX_SIZE; i++) {
            data[addr + i] = buffer[i];
        }
    }

    return 0;
}