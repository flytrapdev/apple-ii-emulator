#include <iostream>
#include <fstream>

#include "testmem.hpp"

using byte = unsigned char;
using word = unsigned short;
using uint32 = unsigned int;

void TestMem::clear() {
    for(uint32 i = 0 ; i < MAX_SIZE ; i++) {
        data[i] = 0;
    }
}

// Read byte wrapper
byte TestMem::readByte(uint32 addr) {
    if(addr < MAX_SIZE)
        return data[addr];
    
    return 0;
}

// Read word wrapper
word TestMem::readWord(uint32 addr) {
    if(addr < MAX_SIZE - 1)
        return (data[addr + 1] << 8) | data[addr];

    return 0;
}

// Write byte wrapper
void TestMem::writeByte(uint32 addr, byte b) {
    if(addr < MAX_SIZE)
        data[addr] = b;
}

// Write word wrapper
void TestMem::writeWord(uint32 addr, word w) {
    if(addr < MAX_SIZE - 1) {
        data[addr] = w & 0x00ff;
        data[addr+1] = (w & 0xff00) >> 8;
    }
}

byte* TestMem::getAddr(uint32 addr) {
    if(addr >= MAX_SIZE)
        return data;

    return data + addr;
}


// Load binary file into memory
int TestMem::loadFile(std::string filename, word addr, bool aux) {

    if(aux) {
        return 0;
    }

    char buffer[MAX_SIZE];

    std::ifstream in;

    in.open(filename, std::ios::in | std::ios::binary);

    if(!in.is_open()) {
        std::cout << "Could not read test ROM file" << std::endl;
        return 1;
    }

    in.read(buffer, MAX_SIZE);

    uint32 size = in.gcount();

    std::cout << "Test ROM size is " << std::dec << (int)in.gcount() << " bytes" << std::endl;

    for(uint32 i = 0; i < size && (i + addr < MAX_SIZE); i++) {
        writeByte(addr + i, buffer[i]);
    }

    return 0;
}