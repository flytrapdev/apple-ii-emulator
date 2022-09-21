/**
 * Memory structure without soft switches
 * Used for CPU testing
 */

#ifndef TESTMEM_HPP
#define TESTMEM_HPP

#include <fstream>
#include "types.hpp"
#include "mem.hpp"


struct TestMem : public Mem {

    constexpr static uint32 MAX_SIZE = 64 * 1024;

    byte data[MAX_SIZE];

    void clear();

    byte readByte(uint32 addr);
    word readWord(uint32 addr);

    void writeByte(uint32 addr, byte b);
    void writeWord(uint32 addr, word w);

    byte* getAddr(uint32 addr);

    int loadFile(std::string filename, word addr, bool aux);
};

#endif