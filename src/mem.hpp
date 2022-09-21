#ifndef MEM_HPP
#define MEM_HPP

#include <fstream>
#include "types.hpp"
#include "disk_drive.hpp"

struct Mem {

    // Max RAM size
    constexpr static uint32 MAX_SIZE = 64 * 1024;

    // Disk drive
    Disk *disk;

    // RAM
    byte data[MAX_SIZE];

    // Auxiliary / bankswitched memory
    byte auxData[MAX_SIZE];
    
    // Soft switches
    //                      OFF  /   ON
    byte sw_80store;    // $C000 / $C001 W
    byte sw_ramrd;      // $C002 / $C003 W
    byte sw_ramwrt;     // $C004 / $C005 W
    byte sw_intcxrom;   // $C006 / $C007 W
    byte sw_altzp;      // $C008 / $C009 W
    byte sw_slotc3rom;  // $C00A / $C00B W
    byte sw_80col;      // $C00C / $C00D W
    byte sw_altcharset; // $C00D / $C00E W

    byte sw_text;       // $C050 / $C051 RW
    byte sw_mixed;      // $C052 / $C053 RW
    byte sw_page2;      // $C054 / $C055 RW
    byte sw_hires;      // $C056 / $C057 RW

    byte sw_an0;        // $C058 / $C059 RW
    byte sw_an1;        // $C05A / $C05B RW
    byte sw_an2;        // $C05C / $C05D RW
    byte sw_an3;        // $C05E / $C05F RW

    // Keyboard data
    byte keyboardKey = 0;

    // Clear keyboard strobe
    void strobeKeyboardKey(byte ascii);
    void clearKeyboardStrobe();

    // Consutrctor
    Mem();

    // Clear RAM
    void init();

    // Internal read/write with soft switches
    byte doRead(uint32 addr);
    void doWrite(uint32 addr, byte value);

    // Read
    byte readByte(uint32 addr);
    word readWord(uint32 addr);

    // Write
    void writeByte(uint32 addr, byte b);
    void writeWord(uint32 addr, word w);

    // Returns pointer to RAM location
    byte* getAddr(uint32 addr);

    // Load ROM file in memory
    int loadFile(std::string filename, word addr, bool aux);
};

#endif