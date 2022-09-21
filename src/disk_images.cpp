#include "disk_images.hpp"
#include <iostream>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>

byte sixAndTwo[0x40] = {
    0x96, 0x97, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F, 0xA6, 0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xCB, 0xCD, 0xCE, 0xCF, 0xD3,
    0xD6, 0xD7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE5, 0xE6, 0xE7, 0xE9, 0xEA, 0xEB, 0xEC,
    0xED, 0xEE, 0xEF, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

byte sectorNumber[3][0x10] = {
    {0x00, 0x08, 0x01, 0x09, 0x02, 0x0A, 0x03, 0x0B, 0x04, 0x0C, 0x05, 0x0D, 0x06, 0x0E, 0x07, 0x0F},
    {0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04, 0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
}; 

// Track and sector location
int diskAddr(byte track, byte sector) {
    return (track * 16 + sector) * 256;
}


// 4-and-4 encoding
void encode44(byte b, byte *output) {
    output[0] = ((b >> 1) & 0x55) | 0xaa;
    output[1] = (b & 0x55) | 0xaa;
}

// Encode a 256-byte sector into a 342 6-bit bytes
void encode62(byte* data, byte* output, int sector) {

    // Result buffer containing 342+1 6-bit bytes
    byte buffer[343];

    // The first 86 bytes contain the lowest 2 bits of all source bytes
    for(int i = 0; i < 84 ; i++) {
        buffer[i] = (REVERSE_BITS(data[i + 172]) << 4) |
                    (REVERSE_BITS(data[i + 86]) << 2) | 
                    (REVERSE_BITS(data[i])); 
    }

    for(int i = 84 ; i < 86 ; i++) {
        buffer[i] = (REVERSE_BITS(data[i + 86]) << 2) |
                    (REVERSE_BITS(data[i]));
    }

    // The final 256 bytes contain the highest 6 bits of all source bytes
    for(int i = 86 ; i < 342 ; i ++) {
        buffer[i] = ((data[i - 86] & 0b11111100) >> 2);
    }

    buffer[342] = buffer[341];

    // XOR the entire data block with itself offset by one byte
    byte result[343];
    result[0] = buffer[0];

    for(int i = 1 ; i < 342 ; i++) {
        result[i] = buffer[i] ^ buffer[i - 1];
    }

    // 343rd byte is the last source byte
    result[342] = buffer[342];

    // Encode 6-bit bytes into disk bytes
    for(int i = 0 ; i < 343 ; i++) {
        output[i] = sixAndTwo[result[i]];
    }

}

// Encode disk track as nibbles
int encodeNibbles(byte* data, byte* output, bool dosOrder, int track) {

    int pos = 0;
    byte volume = 0xfe;

    // Sectors
    for(byte sector = 0 ; sector < 16 ; sector ++) {
        
        // TODO this gives similar results to dsk2nib but seems strange. Verify that this is the correct way to do it.
        byte physicalSector = sectorNumber[dosOrder ? 1 : 0][sector];

        // Gap 1 contains 48 self-sync bytes
        for(int i = 0 ; i < 48 ; i++)
            output[pos++] = 0xff;
        
        // Sector header

        // Prologue
        output[pos++] = 0xd5;
        output[pos++] = 0xaa;
        output[pos++] = 0x96;

        encode44(volume, output+pos);
        encode44((byte)track, output+pos + 2);
        encode44(sector, output+pos + 4);
        encode44(volume ^ ((byte)track) ^ sector, output+pos + 6);
        pos += 8;

        // Epilogue
        output[pos++] = 0xde;
        output[pos++] = 0xaa;
        output[pos++] = 0xeb;

        // Gap 2 contains 6 self-sync bytes
        for(int i = 0 ; i < 5 ; i++)
            output[pos++] = 0xff;
        
        // Sector data

        // Prologue
        output[pos++] = 0xd5;
        output[pos++] = 0xaa;
        output[pos++] = 0xad;

        // Data
        encode62(data + physicalSector * 256, output + pos, physicalSector);
        pos += 343;

        // Epilogue
        output[pos++] = 0xde;
        output[pos++] = 0xaa;
        output[pos++] = 0xeb;

        // Gap 3 contains 27 self-sync bytes
        //for(int i = 0 ; i < 48 ; i++) {
        //    output[pos++] = 0xff;
        //}

    }

    return pos;    
}

int DiskImage::loadFile(std::string filename) {

    std::cout << "Loading disk " << filename << std::endl;

    int fd = open(filename.c_str(), O_RDONLY);

    if(fd == -1) {
        std::cout << "[ERROR] Could not read file " << filename << std::endl;
        return -1;
    }

    int len;
    int pos = 0;

    byte b;

    len = read(fd, &b, 1);

    while(len > 0) {

        if(pos >= DISK_MAXSIZE) {
            std::cout << "ERROR : disk file is too large. Expected " << std::dec << (int)DISK_MAXSIZE << " bytes, disk file is larger." << std::endl;
            close(fd);
            return 1;
        }

        diskFile[pos] = b;
        len = read(fd, &b, 1);
        pos ++;
    }

    if(pos < DISK_MAXSIZE) {
        std::cout << "ERROR : disk file is too small. Expected " << std::dec << (int)DISK_MAXSIZE << " bytes, got " << (int)pos << std::endl;
        close(fd);
        return 1;
    }

    close(fd);

    std::cout << "Disk loaded : " << std::dec << (int)pos << " bytes" << std::endl;

    loaded = true;

    return 0;

}