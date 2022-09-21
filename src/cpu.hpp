#ifndef CPU_HPP
#define CPU_HPP

#include "types.hpp"
#include "mem.hpp"

#define MOS6505_STACK 0x0100
#define MOS6502_NMI 0xfffa
#define MOS6502_RESET 0xfffc
#define MOS6502_IRQ_BRK 0xfffe

struct CPU {

    // Constructor
    CPU(Mem* mem);

    // Registers
    byte a, x, y;

    // Status flag bits
    byte n, v, b, d, i, z, c;

    // Stack pointer
    byte sp;

    // Program counter
    word pc;

    // RAM
    Mem* mem;

    // Interrupts pending
    bool pendingIRQ;
    bool pendingNMI;

    // Previous opcode
    byte currentOpcode;

    // Full-speed mode
    bool ignoreCycles;

    // Number of cycles to emulate
    long cycles;

    // Cycle count for each instruction
    int cycleCount[256] = {
    //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
        7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0, // 0
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 1
        6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0, // 2
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 3
        6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0, // 4
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 5
        6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0, // 6
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // 7
        0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0, // 8
        2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0, // 9
        2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0, // a
        2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0, // b
        2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // c
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0, // d
        2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0, // e
        2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0  // f
    };

    // Instructions with an extra cycle for page boundary cross
    int pageCrossOpcodes[256] = {
    //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // 1
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // 3
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // 5
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // 7
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // a
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, // b
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // c
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, // d
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // e
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0  // f
    };

    // TODO used for debugging
    bool debug = false;

    // Reset CPU
    void reset();

    // Cycles
    void decrementCycles(int cycles);
    bool checkPageCrossed(word addr, int offset);

    // Read memory
    byte readByte(uint32 addr);
    word readWord(uint32 addr);

    void writeByte(uint32 addr, byte b);
    void writeWord(uint32 addr, word w);

    byte nextByte();
    word nextWord();

    // Load ROM file
    int loadFile(std::string filename, word addr, bool aux);

    // Adressing modes
    byte zeropage(byte offset);
    word absolute(word offset);
    word indirect();
    word indexedIndirect();
    word indirectIndexed();

    // BCD conversion
    byte binaryToDecimal(byte bin);
    byte decimalToBinary(byte bin);

    // Set flags
    void setCarry(bool carry);
    void setOverflow(bool overflow);
    void setNegative(bool negative);
    void setDecimal(bool decimal);
    void setZero(bool zero);
    void setBreak(bool brk);
    void setInterrupt(bool in);

    // Update zero and negative flags
    void setAccZ();
    void setAccN();
    void setAccNZ();
    void setNZ(byte value);

    // Update PC
    void setPC(word pc);

    // Get and set status register
    byte getFlagRegister();
    void setFlagRegister(byte flags);

    // Push, pop
    void pushWord(word value);
    word popWord();

    void pushByte(byte value);
    byte popByte();

    // Emulate instructions
    void emulateInstruction();
    void emulateCycles(long cyclesToEmulate);

    // Print information
    void printOpcode(word opcode);
    void printRegisters();

    // Interrupt requests
    void requestIRQ();
    void requestNMI();

    // Interrupt handling
    void handleIRQ();
    void handleNMI();

    // Instructions
    void adc(byte operand);
    void and_(byte operand);
    void asl(byte* operandAddr);
    void bit(byte operand);
    void branch(signed_byte offset);
    void condBranch(byte flag);

    void brk();

    void cmp(byte reg, byte operand);

    void dec(byte* addr);

    void eor(byte operand);

    void inc(byte *addr);

    void jmp(word addr);
    void jsr(word addr);

    void ld(byte* reg, byte operand);

    void lsr(byte* operandAddr);

    void ora(byte operand);

    void rol(byte *operandAddr);
    void ror(byte *operandAddr);

    void rti();
    void rts();

    void sbc(byte operand);

};

#endif