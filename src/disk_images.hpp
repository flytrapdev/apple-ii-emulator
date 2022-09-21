#include "types.hpp"
#include <string>
#include <fstream>

#ifndef DISKIMAGES_HPP
#define DISKIMAGES_HPP

#define DISK_MAXSIZE 143360
#define NIBBLES_PER_TRACK 6656


extern byte sixAndTwo[0x40];
extern byte sectorNumber[3][0x10]; 

int diskAddr(byte track, byte sector);
void encode44(byte b, byte *output);
void encode62(byte* data, byte *output, int sector);
int encodeNibbles(byte* data, byte *output, bool dosOrder, int track);

struct DiskImage {

    byte diskFile[DISK_MAXSIZE];
    bool loaded = false;


    int loadFile(std::string filename);

};

#endif