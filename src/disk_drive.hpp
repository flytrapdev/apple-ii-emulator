#ifndef DISKDRIVE_HPP
#define DISKDRIVE_HPP

#include "types.hpp"
#include "disk_images.hpp"

struct Disk {

    Disk();

    // Disk bootstrap ROM
    // Loads the first sector of the disk in memory and executes it
    byte bootstrapROM[256] =
                    "\xA2\x20\xA0\x00\xA2\x03\x86\x3C\x8A\x0A\x24\x3C\xF0\x10\x05\x3C"
                    "\x49\xFF\x29\x7E\xB0\x08\x4A\xD0\xFB\x98\x9D\x56\x03\xC8\xE8\x10"
                    "\xE5\x20\x58\xFF\xBA\xBD\x00\x01\x0A\x0A\x0A\x0A\x85\x2B\xAA\xBD"
                    "\x8E\xC0\xBD\x8C\xC0\xBD\x8A\xC0\xBD\x89\xC0\xA0\x50\xBD\x80\xC0"
                    "\x98\x29\x03\x0A\x05\x2B\xAA\xBD\x81\xC0\xA9\x56\x20\xA8\xFC\x88"
                    "\x10\xEB\x85\x26\x85\x3D\x85\x41\xA9\x08\x85\x27\x18\x08\xBD\x8C"
                    "\xC0\x10\xFB\x49\xD5\xD0\xF7\xBD\x8C\xC0\x10\xFB\xC9\xAA\xD0\xF3"
                    "\xEA\xBD\x8C\xC0\x10\xFB\xC9\x96\xF0\x09\x28\x90\xDF\x49\xAD\xF0"
                    "\x25\xD0\xD9\xA0\x03\x85\x40\xBD\x8C\xC0\x10\xFB\x2A\x85\x3C\xBD"
                    "\x8C\xC0\x10\xFB\x25\x3C\x88\xD0\xEC\x28\xC5\x3D\xD0\xBE\xA5\x40"
                    "\xC5\x41\xD0\xB8\xB0\xB7\xA0\x56\x84\x3C\xBC\x8C\xC0\x10\xFB\x59"
                    "\xD6\x02\xA4\x3C\x88\x99\x00\x03\xD0\xEE\x84\x3C\xBC\x8C\xC0\x10"
                    "\xFB\x59\xD6\x02\xA4\x3C\x91\x26\xC8\xD0\xEF\xBC\x8C\xC0\x10\xFB"
                    "\x59\xD6\x02\xD0\x87\xA0\x00\xA2\x56\xCA\x30\xFB\xB1\x26\x5E\x00"
                    "\x03\x2A\x5E\x00\x03\x2A\x91\x26\xC8\xD0\xEE\xE6\x27\xE6\x3D\xA5"
                    "\x3D\xCD\x00\x08\xA6\x2B\x90\xDB\x4C\x01\x08\x00\x00\x00\x00";

    int track;              // Current disk track
    int sector;             // Current sector
    int byteCount;          // 6-bit byte index on disk

    bool writeMode;         // Read/write mode

    int motorPhase;         // Current stepper motor track * 2

    bool magnet[4];         // Magnets

    byte currentDrive;      // Selected drive

    bool driveOn[2];        // Drive status

    int curr_byte;          // TODO unused
    byte shiftRegister;     // TODO unused

    bool encoded;           // Check if disk data is encoded

    // TODO add write protection

    int spinning;

    DiskImage *diskImage;

    byte nibbles[35][NIBBLES_PER_TRACK] = {0};

    int loadFile(std::string filename);

    void encodeAll();

    byte diskRead(word addr);
    void diskWrite(word addr, byte val);

    byte setPhase(byte phase, bool on, word addr);
    byte selectDrive(byte drive);
    byte enableDrive(bool enabled);
    byte readWriteData();
    byte setReadMode();
    byte setWriteMode();
};

#endif