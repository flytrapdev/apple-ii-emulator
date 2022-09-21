#include "cpu.hpp"
#include <iostream>


// Interrupt requests

void CPU::requestIRQ() {
    if(!i) {
        pendingIRQ = true;
    }
}

void CPU::requestNMI() {
    pendingNMI = true;
}


// Interrupt handling

void CPU::handleIRQ() {
    pendingIRQ = false;
    setBreak(false);            //B flag is pushed as 0
    pushWord(pc);
    pushByte(getFlagRegister());
    setInterrupt(true);         //disable interrupts after pushing flags
    setPC(readWord(MOS6502_IRQ_BRK));
    
    decrementCycles(7);
}

void CPU::handleNMI() {
    pendingNMI = false;
    setBreak(false);            //B flag is pushed as 0
    pushWord(pc);
    pushByte(getFlagRegister());
    setInterrupt(true);         //disable interrupts after pushing flags
    setPC(readWord(MOS6502_NMI));

    decrementCycles(7);
}


// Individual instructions

void CPU::adc(byte operand) {

    byte acc = a;
    
    if(d) {
        //BCD mode
        operand = binaryToDecimal(operand);
        acc = binaryToDecimal(a);
    }
    
    uint32 result = acc + operand + c;

    if(d) {
        //Overflow flag not affected in decimal mode
        setCarry(result > 99);

        if(result > 99) {
            result -= 100;
        }

        a = decimalToBinary(result);
    }
    else {
        int signedResult = (signed char)acc + (signed char)operand + c;

        setOverflow((signedResult > 127 || signedResult < -128));
        setCarry(result > 0xff);

        a = result & 0xff;
    }
    
    setAccNZ();
}

void CPU::and_(byte operand) {
    a &= operand;

    setAccNZ();
}

void CPU::asl(byte* operandAddr) {
    setCarry((*operandAddr) >> 7); // Bit 7 shifted into carry
    *operandAddr <<= 1;            // Shift bits left

    setNZ((*operandAddr));
}

void CPU::branch(signed_byte offset) {
    setPC(pc + offset);
}

void CPU::condBranch(byte flag) {
    signed_byte offset = nextByte();

    if(flag) {
        decrementCycles(1);     // 1 extra cycle if branch is taken

        if(checkPageCrossed(pc, offset))
            decrementCycles(1); // Another cycle if 256 byte boundary is crossed

        branch(offset);
    }
}


void CPU::bit(byte operand) {
    byte result = a & operand;

    setNegative((operand >> 7) != 0);
    setOverflow(((operand & 0x40) >> 6) != 0);
    setZero((result == 0));
}

void CPU::brk() {
    nextByte();             //Read one byte after BRK
    setBreak(true);         //BRK pushes B flag as 1
    pushWord(pc);
    pushByte(getFlagRegister());
    setInterrupt(true);     //disable interrupts after pushing flags
    setPC(readWord(MOS6502_IRQ_BRK));
}

void CPU::cmp(byte reg, byte operand) {
    byte result = reg - operand;

    setCarry(reg >= operand);
    setZero(reg == operand);

    setNegative(result & 0x80);
}

void CPU::dec(byte* addr) {
    (*addr) --;
    setNZ((*addr));
}

void CPU::eor(byte operand) {
    a ^= operand;

    setAccNZ();
}

void CPU::inc(byte* addr) {
    (*addr) ++;
    setNZ((*addr));
}

void CPU::jmp(word addr) {
    //TODO : bug on original chips in indirect mode
    setPC(addr);
}

void CPU::jsr(word addr) {
    pushWord(pc - 1);
    setPC(addr);
}

void CPU::ld(byte* reg, byte operand) {
    *reg = operand;

    setNZ((*reg));
}

void CPU::lsr(byte* operandAddr) {
    setCarry((*operandAddr) & 0x01);

    *operandAddr >>= 1;

    setNZ((*operandAddr));
}

void CPU::ora(byte operand) {
    a |= operand;

    setAccNZ();
}

void CPU::rol(byte* operandAddr) {
    byte carry = c;

    setCarry((*operandAddr) >> 7);

    *operandAddr = (*operandAddr << 1) | carry;

    setNZ((*operandAddr));
}

void CPU::ror(byte* operandAddr) {
    byte carry = c;

    setCarry((*operandAddr) & 0x01);

    *operandAddr = (*operandAddr >> 1) | (carry << 7);

    setNZ((*operandAddr));
}

void CPU::rti() {
    setFlagRegister(popByte());
    setPC(popWord());
}

void CPU::rts() {
    setPC(popWord() + 1);
}

void CPU::sbc(byte operand) {
    if(!d)
        adc(~operand);
    else {
        operand = binaryToDecimal(operand);
        byte acc = binaryToDecimal(a);

        int result = acc - operand - (1 - c);

        setCarry(result >= 0);

        if(result < 0) {
            result += 100;
        }

        a = decimalToBinary(result);

        setAccNZ();
    }
}