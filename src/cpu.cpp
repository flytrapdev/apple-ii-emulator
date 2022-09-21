#include "cpu.hpp"
#include <iostream>
#include <iomanip>
#include <bitset>

CPU::CPU(Mem* mem) {
    // RAM
    this->mem = mem;

    ignoreCycles = false;
}

void CPU::reset() {

    // No cycles to emulate
    cycles = 0;

    // Clear registers
    a = 0;
    x = 0;
    y = 0;

    // Flag I is set
    setFlagRegister(0);
    i = 1;

    // Load ROMs into memory
    mem->init();

    // Stack pointer starts at 0xfd
    sp = 0xfd;

    // Jump to reset vector
    pc = readWord(MOS6502_RESET);

    std::cout << "Reset vector points to " << std::hex << pc << std::endl;
}

void CPU::decrementCycles(int cycles) {
    if(!ignoreCycles)
        this->cycles -= cycles;
}

bool CPU::checkPageCrossed(word addr, int offset) {
    return (((addr + offset) & 0xff00) != (addr & 0xff00));
}

// Memory function wrappers

byte CPU::readByte(uint32 addr) { return mem->readByte(addr); }
word CPU::readWord(uint32 addr) { return mem->readWord(addr); }

void CPU::writeByte(uint32 addr, byte b) { mem->writeByte(addr, b); }
void CPU::writeWord(uint32 addr, word w) { mem->writeWord(addr, w); }

byte CPU::nextByte() {
    byte b = mem->readByte(pc);
    pc ++;
    return b;
}

word CPU::nextWord() {
    word w = mem->readWord(pc);
    pc += 2;
    return w;
}

int CPU::loadFile(std::string filename, word addr, bool aux) {
    return mem->loadFile(filename, addr, aux);
}

// Zero-page addressing mode
byte CPU::zeropage(byte offset) { return nextByte() + offset; }

// Absolute addressing mode
word CPU::absolute(word offset) {

    word next = nextWord();

    // Extra cycle for page boundary cross
    if(!ignoreCycles && checkPageCrossed(next, offset))
        decrementCycles(1);


    return next + offset;
}

// Indirect addressing for JMP instruction
word CPU::indirect() {
    return mem->readWord(nextWord());
}

// Indexed indirect addressing
word CPU::indexedIndirect() {
    return mem->readWord((nextByte() + x) & 0xff);
}

// Indirect indexed addressing
word CPU::indirectIndexed() {

    byte next = nextByte();

    // Extra cycle for page boundary cross
    if(!ignoreCycles && checkPageCrossed(mem->readWord(next), y))
        decrementCycles(1);

    return mem->readWord(next) + y;
}

// BCD conversions
byte CPU::binaryToDecimal(byte bin) {
    return (bin & 0x0f) + ((bin & 0xf0) >> 4) * 10;
}

byte CPU::decimalToBinary(byte dec) {
    return ((dec / 10) << 4 | (dec % 10));
}

// CPU flags
void CPU::setBreak(bool brk) { b = brk ? 1 : 0; }
void CPU::setCarry(bool carry) { c = carry ? 1 : 0; }
void CPU::setDecimal(bool decimal) { d = decimal ? 1 : 0; }
void CPU::setNegative(bool negative) { n = negative ? 1 : 0; }
void CPU::setOverflow(bool overflow) { v = overflow ? 1 : 0; }
void CPU::setZero(bool zero) { z = zero ? 1 : 0; }
void CPU::setInterrupt(bool in) { i = in ? 1 : 0; }

// Update N and Z flags
void CPU::setNZ(byte value) {
    setNegative((value & 0x80) != 0);
    setZero(value == 0);
}

void CPU::setAccN() { setNegative((a & 0x80) != 0); }
void CPU::setAccZ() { setZero(a == 0); }

void CPU::setAccNZ() { setNZ(a); }

// Program counter
void CPU::setPC(word pc) { this->pc = pc; }

// Flag / status register
byte CPU::getFlagRegister() {
    return (n << 7) | (v << 6) | (1 << 5) | (b << 4) | (d << 3) | (i << 2) | (z << 1) | c;
}

void CPU::setFlagRegister(byte flags) {
    n = (flags >> 7) ;
    v = (flags >> 6) & 1;
    b = (flags >> 4) & 1;
    d = (flags >> 3) & 1;
    i = (flags >> 2) & 1;
    z = (flags >> 1) & 1;
    c = (flags) & 1;
}

// Stack push / pop
void CPU::pushWord(word value) {
    mem->writeWord(MOS6505_STACK + sp - 1, value);
    sp -= 2;
}

word CPU::popWord() {
    sp += 2;
    return mem->readWord(MOS6505_STACK + sp - 1);
}

void CPU::pushByte(byte value) {
    mem->writeByte(MOS6505_STACK + sp, value);
    sp --;
}

byte CPU::popByte() {
    sp ++;
    return mem->readByte(MOS6505_STACK + sp);
}

// Emulate a certain numbre of cycles
void CPU::emulateCycles(long cyclesToEmulate) {
    // Add new cycles to emulate
    cycles += cyclesToEmulate;

    while (cycles > 0) {
        //printOpcode(readWord(pc));
        emulateInstruction();
    }
}

// Emulate the next CPU instruction
void CPU::emulateInstruction() {

    // NMI pending
    if(pendingNMI) {
        handleNMI();
        return;
    }

    // IRQ pending
    if(pendingIRQ) {
        handleIRQ();
        return;
    }

    if(debug)
        printOpcode(pc);
        

    // Fetch opcode 
    byte opcode = nextByte();

    // Cycle count
    decrementCycles(cycleCount[opcode]);

    currentOpcode = opcode;

    switch(opcode) {

        // ADC
        case 0x69: adc(nextByte()); break;                                                  //ADC #d8
        case 0x65: adc(readByte(zeropage(0))); break;                                       //ADC a8
        case 0x75: adc(readByte(zeropage(x))); break;                                       //ADC a8,X
        case 0x6d: adc(readByte(absolute(0))); break;                                       //ADC a16
        case 0x7d: adc(readByte(absolute(x))); break;                                       //ADC a16,X
        case 0x79: adc(readByte(absolute(y))); break;                                       //ADC a16,Y
        case 0x61: adc(readByte(indexedIndirect())); break;                                 //ADC (a8,X)
        case 0x71: adc(readByte(indirectIndexed())); break;                                 //ADC (a8),Y

        // AND
        case 0x29: and_(nextByte()); break;                                                 //AND #d8
        case 0x25: and_(readByte(zeropage(0))); break;                                      //AND a8
        case 0x35: and_(readByte(zeropage(x))); break;                                      //AND a8,X
        case 0x2d: and_(readByte(absolute(0))); break;                                      //AND a16
        case 0x3d: and_(readByte(absolute(x))); break;                                      //AND a16,X
        case 0x39: and_(readByte(absolute(y))); break;                                      //AND a16,Y
        case 0x21: and_(readByte(indexedIndirect())); break;                                //AND (a8,X)
        case 0x31: and_(readByte(indirectIndexed())); break;                                //AND (a8),Y

        // ASL
        case 0x0a: asl(&a); break;                                                          //ASL a
        case 0x06: asl(mem->getAddr(zeropage(0))); break;                                   //ASL a8
        case 0x16: asl(mem->getAddr(zeropage(x))); break;                                   //ASL a8,X
        case 0x0e: asl(mem->getAddr(absolute(0))); break;                                   //ASL a16
        case 0x1e: asl(mem->getAddr(absolute(x))); break;                                   //ASL a16,X

        // BIT
        case 0x24: bit(readByte(zeropage(0))); break;                                       //BIT a8
        case 0x2c: bit(readByte(absolute(0))); break;                                       //BIT a16

        // Branches
        case 0x90: condBranch(!c); break;                                                   //BCC
        case 0xb0: condBranch(c); break;                                                    //BCS
        case 0xf0: condBranch(z); break;                                                    //BEQ
        case 0x30: condBranch(n); break;                                                    //BMI
        case 0xd0: condBranch(!z); break;                                                   //BNE
        case 0x10: condBranch(!n); break;                                                   //BPL
        case 0x50: condBranch(!v); break;                                                   //BVC
        case 0x70: condBranch(v); break;                                                    //BVS

        // BRK
        case 0x00: brk(); break;                                                            //BRK

        // Clear flags
        case 0x18: c = 0; break;                                                            //CLC
        case 0xd8: d = 0; break;                                                            //CLD
        case 0x58: i = 0; break;                                                            //CLI
        case 0xb8: v = 0; break;                                                            //CLV

        // CMP
        case 0xc9: cmp(a, nextByte()); break;                                               //CMP #d8
        case 0xc5: cmp(a, readByte(zeropage(0))); break;                                    //CMP a8
        case 0xd5: cmp(a, readByte(zeropage(x))); break;                                    //CMP a8,X
        case 0xcd: cmp(a, readByte(absolute(0))); break;                                    //CMP a16
        case 0xdd: cmp(a, readByte(absolute(x))); break;                                    //CMP a16,X
        case 0xd9: cmp(a, readByte(absolute(y))); break;                                    //CMP a16,Y
        case 0xc1: cmp(a, readByte(indexedIndirect())); break;                              //CMP (a8,X)
        case 0xd1: cmp(a, readByte(indirectIndexed())); break;                              //CMP (a8),Y

        // CPX
        case 0xe0: cmp(x, nextByte()); break;                                               //CPX #d8
        case 0xe4: cmp(x, readByte(zeropage(0))); break;                                    //CPX a8
        case 0xec: cmp(x, readByte(absolute(0))); break;                                    //CPX a16

        // CPY
        case 0xc0: cmp(y, nextByte()); break;                                               //CPY #d8
        case 0xc4: cmp(y, readByte(zeropage(0))); break;                                    //CPY a8
        case 0xcc: cmp(y, readByte(absolute(0))); break;                                    //CPY a16

        // DEC
        case 0xc6: dec(mem->getAddr(zeropage(0))); break;                                   //DEC a8
        case 0xd6: dec(mem->getAddr(zeropage(x))); break;                                   //DEC a8,X
        case 0xce: dec(mem->getAddr(absolute(0))); break;                                   //DEC a16
        case 0xde: dec(mem->getAddr(absolute(x))); break;                                   //DEC a16,X

        // DEX
        case 0xca: dec(&x); break;                                                          //DEX

        // DEY
        case 0x88: dec(&y); break;                                                          //DEY

        // EOR
        case 0x49: eor(nextByte()); break;                                                  //EOR #d8
        case 0x45: eor(readByte(zeropage(0))); break;                                       //EOR a8
        case 0x55: eor(readByte(zeropage(x))); break;                                       //EOR a8,X
        case 0x4d: eor(readByte(absolute(0))); break;                                       //EOR a16
        case 0x5d: eor(readByte(absolute(x))); break;                                       //EOR a16,X
        case 0x59: eor(readByte(absolute(y))); break;                                       //EOR a16,Y
        case 0x41: eor(readByte(indexedIndirect())); break;                                 //EOR (a8,X)
        case 0x51: eor(readByte(indirectIndexed())); break;                                 //EOR (a8),Y

        // INC
        case 0xe6: inc(mem->getAddr(zeropage(0))); break;                                   //INC a8
        case 0xf6: inc(mem->getAddr(zeropage(x))); break;                                   //INC a8,X
        case 0xee: inc(mem->getAddr(absolute(0))); break;                                   //INC a16
        case 0xfe: inc(mem->getAddr(absolute(x))); break;                                   //INC a16,X

        // INX
        case 0xe8: inc(&x); break;                                                          //INX

        // INY
        case 0xc8: inc(&y); break;                                                          //INY

        // JMP
        case 0x4c: jmp(nextWord()); break;                                                  //JMP a16
        case 0x6c: jmp(indirect()); break;                                                  //JMP (a16)

        // JSR
        case 0x20: jsr(absolute(0)); break;                                                 //JSR a16

        // LDA
        case 0xa9: ld(&a, nextByte()); break;                                               //LDA #d8
        case 0xa5: ld(&a, readByte(zeropage(0))); break;                                    //LDA a8
        case 0xb5: ld(&a, readByte(zeropage(x))); break;                                    //LDA a8,X
        case 0xad: ld(&a, readByte(absolute(0))); break;                                    //LDA a16
        case 0xbd: ld(&a, readByte(absolute(x))); break;                                    //LDA a16,X
        case 0xb9: ld(&a, readByte(absolute(y))); break;                                    //LDA a16,Y
        case 0xa1: ld(&a, readByte(indexedIndirect())); break;                              //LDA (a8,X)
        case 0xb1: ld(&a, readByte(indirectIndexed())); break;                              //LDA (a8),Y

        // LDX
        case 0xa2: ld(&x, nextByte()); break;                                               //LDX #d8
        case 0xa6: ld(&x, readByte(zeropage(0))); break;                                    //LDX a8
        case 0xb6: ld(&x, readByte(zeropage(y))); break;                                    //LDX a8,Y
        case 0xae: ld(&x, readByte(absolute(0))); break;                                    //LDX a16
        case 0xbe: ld(&x, readByte(absolute(y))); break;                                    //LDX a16,Y

        // LDY
        case 0xa0: ld(&y, nextByte()); break;                                               //LDY #d8
        case 0xa4: ld(&y, readByte(zeropage(0))); break;                                    //LDY a8
        case 0xb4: ld(&y, readByte(zeropage(x))); break;                                    //LDY a8,X
        case 0xac: ld(&y, readByte(absolute(0))); break;                                    //LDY a16
        case 0xbc: ld(&y, readByte(absolute(x))); break;                                    //LDY a16,X

        // LSR
        case 0x4a: lsr(&a); break;                                                          //LSR a
        case 0x46: lsr(mem->getAddr(zeropage(0))); break;                                   //LSR a8
        case 0x56: lsr(mem->getAddr(zeropage(x))); break;                                   //LSR a8,X
        case 0x4e: lsr(mem->getAddr(absolute(0))); break;                                   //LSR a16
        case 0x5e: lsr(mem->getAddr(absolute(x))); break;                                   //LSR a16,X

        // NOP
        case 0xea: break;                                                                   //NOP
        case 0x42: nextByte(); break;                                                       //NOP imm (65C02)

        // ORA
        case 0x09: ora(nextByte()); break;                                                  //ORA #d8
        case 0x05: ora(readByte(zeropage(0))); break;                                       //ORA a8
        case 0x15: ora(readByte(zeropage(x))); break;                                       //ORA a8,X
        case 0x0d: ora(readByte(absolute(0))); break;                                       //ORA a16
        case 0x1d: ora(readByte(absolute(x))); break;                                       //ORA a16,X
        case 0x19: ora(readByte(absolute(y))); break;                                       //ORA a16,Y
        case 0x01: ora(readByte(indexedIndirect())); break;                                 //ORA (a8,X)
        case 0x11: ora(readByte(indirectIndexed())); break;                                 //ORA (a8),Y

        // PHA
        case 0x48: pushByte(a); break;                                                      //PHA

        // PHP
        case 0x08: b = 1; pushByte(getFlagRegister()); break;                               //PHP

        // PLA
        case 0x68: a = popByte(); setAccNZ(); break;                                        //PLA

        // PLP
        case 0x28: setFlagRegister(popByte()); break;                                       //PLP

        // ROL
        case 0x2a: rol(&a); break;                                                          //ROL a
        case 0x26: rol(mem->getAddr(zeropage(0))); break;                                   //ROL a8
        case 0x36: rol(mem->getAddr(zeropage(x))); break;                                   //ROL a8,X
        case 0x2e: rol(mem->getAddr(absolute(0))); break;                                   //ROL a16
        case 0x3e: rol(mem->getAddr(absolute(x))); break;                                   //ROL a16,X

        // ROR
        case 0x6a: ror(&a); break;                                                          //ROR a
        case 0x66: ror(mem->getAddr(zeropage(0))); break;                                   //ROR a8
        case 0x76: ror(mem->getAddr(zeropage(x))); break;                                   //ROR a8,X
        case 0x6e: ror(mem->getAddr(absolute(0))); break;                                   //ROR a16
        case 0x7e: ror(mem->getAddr(absolute(x))); break;                                   //ROR a16,X

        // RTI
        case 0x40: rti(); break;                                                            //RTI

        // RTS
        case 0x60: rts(); break;                                                            //RTS

        // SDB
        case 0xe9: sbc(nextByte()); break;                                                  //SBC #d8
        case 0xe5: sbc(readByte(zeropage(0))); break;                                       //SBC a8
        case 0xf5: sbc(readByte(zeropage(x))); break;                                       //SBC a8,X
        case 0xed: sbc(readByte(absolute(0))); break;                                       //SBC a16
        case 0xfd: sbc(readByte(absolute(x))); break;                                       //SBC a16,X
        case 0xf9: sbc(readByte(absolute(y))); break;                                       //SBC a16,Y
        case 0xe1: sbc(readByte(indexedIndirect())); break;                                 //SBC (a8,X)
        case 0xf1: sbc(readByte(indirectIndexed())); break;                                 //SBC (a8),Y

        // Set flags
        case 0x38: c = 1; break;                                                            //SEC
        case 0xf8: d = 1; break;                                                            //SED
        case 0x78: i = 1; break;                                                            //SEI

        // STA
        case 0x85: writeByte(zeropage(0), a); break;                                        //STA a8
        case 0x95: writeByte(zeropage(x), a); break;                                        //STA a8,X
        case 0x8d: writeByte(absolute(0), a); break;                                        //STA a16
        case 0x9d: writeByte(absolute(x), a); break;                                        //STA a16,X
        case 0x99: writeByte(absolute(y), a); break;                                        //STA a16,Y
        case 0x81: writeByte(indexedIndirect(), a); break;                                  //STA (a8,X)
        case 0x91: writeByte(indirectIndexed(), a); break;                                  //STA (a8),Y

        // STX
        case 0x86: writeByte(zeropage(0), x); break;                                        //STX a8
        case 0x96: writeByte(zeropage(y), x); break;                                        //STX a8,Y
        case 0x8e: writeByte(absolute(0), x); break;                                        //STX a16

        // STY
        case 0x84: writeByte(zeropage(0), y); break;                                        //STY a8
        case 0x94: writeByte(zeropage(x), y); break;                                        //STY a8,X
        case 0x8c: writeByte(absolute(0), y); break;                                        //STY a16

        // Transfer operations
        case 0xaa: x = a; setAccNZ(); break;                                                //TAX
        case 0xa8: y = a; setAccNZ(); break;                                                //TAY
        case 0xba: x = sp; setNZ(x); break;                                                 //TSX
        case 0x8a: a = x; setAccNZ(); break;                                                //TXA
        case 0x9a: sp = x; break;                                                           //TXS
        case 0x98: a = y; setAccNZ(); break;                                                //TYA

        // Unknown opcode
        default : std::cout << "Unknown opcode: " << std::hex << std::setw(2) << (int)opcode << " at pc " << std::setw(4) << (int) pc << std::endl;
    }
}

// Print A, X, Y, SP and status registers
void CPU::printRegisters() {
    std::cout << std::hex << std::setw(2) << "A:" << (int)a << " X:" << (int)x << " Y:" << (int)y << " SP:" << (int)sp << " flags:" << std::bitset<8>(getFlagRegister()) << std::endl;
}

// Print opcode mnemonic
void CPU::printOpcode(word addr) {

    word opcode = readByte(addr);

    std::cout << std::hex << std::setw(4) << (int)addr << " ";

    switch(opcode) {

        case 0x69: std::cout << std::hex << std::setw(2) << "ADC #" << (int)readByte(addr + 1) << std::endl; break;
        case 0x65: std::cout << std::hex << std::setw(2) << "ADC $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x75: std::cout << std::hex << std::setw(2) << "ADC $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x6d: std::cout << std::hex << std::setw(2) << "ADC $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x7d: std::cout << std::hex << std::setw(2) << "ADC $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0x79: std::cout << std::hex << std::setw(2) << "ADC $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0x61: std::cout << std::hex << std::setw(2) << "ADC ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0x71: std::cout << std::hex << std::setw(2) << "ADC ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0x29: std::cout << std::hex << std::setw(2) << "AND #" << (int)readByte(addr + 1) << std::endl; break;
        case 0x25: std::cout << std::hex << std::setw(2) << "AND $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x35: std::cout << std::hex << std::setw(2) << "AND $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x2d: std::cout << std::hex << std::setw(2) << "AND $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x3d: std::cout << std::hex << std::setw(2) << "AND $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0x39: std::cout << std::hex << std::setw(2) << "AND $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0x21: std::cout << std::hex << std::setw(2) << "AND ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0x31: std::cout << std::hex << std::setw(2) << "AND ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0x0a: std::cout << std::hex << std::setw(2) << "ASL a" << std::endl; break;
        case 0x06: std::cout << std::hex << std::setw(2) << "ASL $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x16: std::cout << std::hex << std::setw(2) << "ASL $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x0e: std::cout << std::hex << std::setw(2) << "ASL $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x1e: std::cout << std::hex << std::setw(2) << "ASL $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0x24: std::cout << std::hex << std::setw(2) << "BIT $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x2c: std::cout << std::hex << std::setw(2) << "BIT $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0x90: std::cout << std::hex << std::setw(2) << "BCC" << std::endl; break;
        case 0xb0: std::cout << std::hex << std::setw(2) << "BCS" << std::endl; break;
        case 0xf0: std::cout << std::hex << std::setw(2) << "BEQ" << std::endl; break;
        case 0x30: std::cout << std::hex << std::setw(2) << "BMI" << std::endl; break;
        case 0xd0: std::cout << std::hex << std::setw(2) << "BNE" << std::endl; break;
        case 0x10: std::cout << std::hex << std::setw(2) << "BPL" << std::endl; break;
        case 0x50: std::cout << std::hex << std::setw(2) << "BVC" << std::endl; break;
        case 0x70: std::cout << std::hex << std::setw(2) << "BVS" << std::endl; break;

        case 0x00: std::cout << std::hex << std::setw(2) << "BRK" << std::endl; break;

        case 0x18: std::cout << std::hex << std::setw(2) << "CLC" << std::endl; break;
        case 0xd8: std::cout << std::hex << std::setw(2) << "CLD" << std::endl; break;
        case 0x58: std::cout << std::hex << std::setw(2) << "CLI" << std::endl; break;
        case 0xb8: std::cout << std::hex << std::setw(2) << "CLV" << std::endl; break;

        case 0xc9: std::cout << std::hex << std::setw(2) << "CMP #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xc5: std::cout << std::hex << std::setw(2) << "CMP $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xd5: std::cout << std::hex << std::setw(2) << "CMP $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xcd: std::cout << std::hex << std::setw(2) << "CMP $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xdd: std::cout << std::hex << std::setw(2) << "CMP $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0xd9: std::cout << std::hex << std::setw(2) << "CMP $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0xc1: std::cout << std::hex << std::setw(2) << "CMP ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0xd1: std::cout << std::hex << std::setw(2) << "CMP ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0xe0: std::cout << std::hex << std::setw(2) << "CPX #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xe4: std::cout << std::hex << std::setw(2) << "CPX $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xec: std::cout << std::hex << std::setw(2) << "CPX $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0xc0: std::cout << std::hex << std::setw(2) << "CPY #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xc4: std::cout << std::hex << std::setw(2) << "CPY $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xcc: std::cout << std::hex << std::setw(2) << "CPY $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0xc6: std::cout << std::hex << std::setw(2) << "DEC $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xd6: std::cout << std::hex << std::setw(2) << "DEC $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xce: std::cout << std::hex << std::setw(2) << "DEC $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xde: std::cout << std::hex << std::setw(2) << "DEC $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0xca: std::cout << std::hex << std::setw(2) << "DEX" << std::endl; break;

        case 0x88: std::cout << std::hex << std::setw(2) << "DEY" << std::endl; break;

        case 0x49: std::cout << std::hex << std::setw(2) << "EOR #" << (int)readByte(addr + 1) << std::endl; break;
        case 0x45: std::cout << std::hex << std::setw(2) << "EOR $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x55: std::cout << std::hex << std::setw(2) << "EOR $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x4d: std::cout << std::hex << std::setw(2) << "EOR $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x5d: std::cout << std::hex << std::setw(2) << "EOR $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0x59: std::cout << std::hex << std::setw(2) << "EOR $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0x41: std::cout << std::hex << std::setw(2) << "EOR ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0x51: std::cout << std::hex << std::setw(2) << "EOR ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0xe6: std::cout << std::hex << std::setw(2) << "INC $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xf6: std::cout << std::hex << std::setw(2) << "INC $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xee: std::cout << std::hex << std::setw(2) << "INC $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xfe: std::cout << std::hex << std::setw(2) << "INC $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0xe8: std::cout << std::hex << std::setw(2) << "INX" << std::endl; break;

        case 0xc8: std::cout << std::hex << std::setw(2) << "INY" << std::endl; break;

        case 0x4c: std::cout << std::hex << std::setw(2) << "JMP $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x6c: std::cout << std::hex << std::setw(2) << "JMP ($" << (int)readWord(addr + 1) << ") " << std::endl; break;

        case 0x20: std::cout << std::hex << std::setw(2) << "JSR $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0xa9: std::cout << std::hex << std::setw(2) << "LDA #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xa5: std::cout << std::hex << std::setw(2) << "LDA $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xb5: std::cout << std::hex << std::setw(2) << "LDA $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xad: std::cout << std::hex << std::setw(2) << "LDA $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xbd: std::cout << std::hex << std::setw(2) << "LDA $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0xb9: std::cout << std::hex << std::setw(2) << "LDA $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0xa1: std::cout << std::hex << std::setw(2) << "LDA ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0xb1: std::cout << std::hex << std::setw(2) << "LDA ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0xa2: std::cout << std::hex << std::setw(2) << "LDX #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xa6: std::cout << std::hex << std::setw(2) << "LDX $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xb6: std::cout << std::hex << std::setw(2) << "LDX $" << (int)readByte(addr + 1) << ",Y" << std::endl; break;
        case 0xae: std::cout << std::hex << std::setw(2) << "LDX $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xbe: std::cout << std::hex << std::setw(2) << "LDX $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;

        case 0xa0: std::cout << std::hex << std::setw(2) << "LDY #" << (int)readByte(addr + 1) << std::endl; break;
        case 0xa4: std::cout << std::hex << std::setw(2) << "LDY $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xb4: std::cout << std::hex << std::setw(2) << "LDY $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xac: std::cout << std::hex << std::setw(2) << "LDY $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xbc: std::cout << std::hex << std::setw(2) << "LDY $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0x4a: std::cout << std::hex << std::setw(2) << "LSR a" << std::endl; break;
        case 0x46: std::cout << std::hex << std::setw(2) << "LSR $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x56: std::cout << std::hex << std::setw(2) << "LSR $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x4e: std::cout << std::hex << std::setw(2) << "LSR $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x5e: std::cout << std::hex << std::setw(2) << "LSR $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0xea: std::cout << std::hex << std::setw(2) << "NOP" << std::endl; break;
        case 0x42: std::cout << std::hex << std::setw(2) << "NOP imm (65C02)" << std::endl; break;

        case 0x09: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readByte(addr + 1) << std::endl; break;
        case 0x05: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x15: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x0d: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x1d: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0x19: std::cout << std::hex << std::setw(2) << "ORA $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0x01: std::cout << std::hex << std::setw(2) << "ORA ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0x11: std::cout << std::hex << std::setw(2) << "ORA ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0x48: std::cout << std::hex << std::setw(2) << "PHA" << std::endl; break;

        case 0x08: std::cout << std::hex << std::setw(2) << "PHP" << std::endl; break;

        case 0x68: std::cout << std::hex << std::setw(2) << "PLA" << std::endl; break;

        case 0x28: std::cout << std::hex << std::setw(2) << "PLP" << std::endl; break;

        case 0x2a: std::cout << std::hex << std::setw(2) << "ROL a" << std::endl; break;
        case 0x26: std::cout << std::hex << std::setw(2) << "ROL $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x36: std::cout << std::hex << std::setw(2) << "ROL $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x2e: std::cout << std::hex << std::setw(2) << "ROL $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x3e: std::cout << std::hex << std::setw(2) << "ROL $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0x6a: std::cout << std::hex << std::setw(2) << "ROR a" << std::endl; break;
        case 0x66: std::cout << std::hex << std::setw(2) << "ROR $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x76: std::cout << std::hex << std::setw(2) << "ROR $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x6e: std::cout << std::hex << std::setw(2) << "ROR $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x7e: std::cout << std::hex << std::setw(2) << "ROR $" << (int)readWord(addr + 1) << ",X" << std::endl; break;

        case 0x40: std::cout << std::hex << std::setw(2) << "RTI" << std::endl; break;
        case 0x60: std::cout << std::hex << std::setw(2) << "RTS" << std::endl; break;

        case 0xe9: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readByte(addr + 1) << std::endl; break;
        case 0xe5: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0xf5: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0xed: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0xfd: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0xf9: std::cout << std::hex << std::setw(2) << "SBC $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0xe1: std::cout << std::hex << std::setw(2) << "SBC ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0xf1: std::cout << std::hex << std::setw(2) << "SBC ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0x38: std::cout << std::hex << std::setw(2) << "SEC" << std::endl; break;
        case 0xf8: std::cout << std::hex << std::setw(2) << "SED" << std::endl; break;
        case 0x78: std::cout << std::hex << std::setw(2) << "SEI" << std::endl; break;

        case 0x85: std::cout << std::hex << std::setw(2) << "STA $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x95: std::cout << std::hex << std::setw(2) << "STA $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x8d: std::cout << std::hex << std::setw(2) << "STA $" << (int)readWord(addr + 1) << "" << std::endl; break;
        case 0x9d: std::cout << std::hex << std::setw(2) << "STA $" << (int)readWord(addr + 1) << ",X" << std::endl; break;
        case 0x99: std::cout << std::hex << std::setw(2) << "STA $" << (int)readWord(addr + 1) << ",Y" << std::endl; break;
        case 0x81: std::cout << std::hex << std::setw(2) << "STA ($" << (int)readByte(addr + 1) << ",X)" << std::endl; break;
        case 0x91: std::cout << std::hex << std::setw(2) << "STA ($" << (int)readByte(addr + 1) << "),Y" << std::endl; break;

        case 0x86: std::cout << std::hex << std::setw(2) << "STX $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x96: std::cout << std::hex << std::setw(2) << "STX $" << (int)readByte(addr + 1) << ",Y" << std::endl; break;
        case 0x8e: std::cout << std::hex << std::setw(2) << "STX $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0x84: std::cout << std::hex << std::setw(2) << "STY $" << (int)readByte(addr + 1) << "" << std::endl; break;
        case 0x94: std::cout << std::hex << std::setw(2) << "STY $" << (int)readByte(addr + 1) << ",X" << std::endl; break;
        case 0x8c: std::cout << std::hex << std::setw(2) << "STY $" << (int)readWord(addr + 1) << "" << std::endl; break;

        case 0xaa: std::cout << std::hex << std::setw(2) << "TAX" << std::endl; break;
        case 0xa8: std::cout << std::hex << std::setw(2) << "TAY" << std::endl; break;
        case 0xba: std::cout << std::hex << std::setw(2) << "TSX" << std::endl; break;
        case 0x8a: std::cout << std::hex << std::setw(2) << "TXA" << std::endl; break;
        case 0x9a: std::cout << std::hex << std::setw(2) << "TXS" << std::endl; break;
        case 0x98: std::cout << std::hex << std::setw(2) << "TYA" << std::endl; break;

        // Unknown opcode
        default : std::cout << "Unknown opcode: " << std::hex << std::setw(2) << (int)opcode << " at pc " << std::setw(4) << (int) pc << std::endl;
    }
}
