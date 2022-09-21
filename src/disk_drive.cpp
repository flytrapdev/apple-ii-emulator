#include "disk_drive.hpp"
#include "disk_images.hpp"
#include <iostream>

#define DISK_LOG 0

Disk::Disk() {

    motorPhase = 0;
    magnet[0] = false;
    magnet[1] = false;
    magnet[2] = false;
    magnet[3] = false;

    driveOn[0] = false;
    driveOn[1] = false;

    sector = 0;
    byteCount = 0;
    track = 0;

    writeMode = false;

    spinning = 0;

    diskImage = new DiskImage();

    encoded = false;
}

int Disk::loadFile(std::string filename) {
    encoded = false;
    return diskImage->loadFile(filename);
}

byte Disk::setPhase(byte phase, bool on, word addr) {

    magnet[phase % 4] = on;

    int direction = 0;

    // Will the stepper motor move ?
    if(!magnet[motorPhase % 4]) {
        if(magnet[(motorPhase + 3) & 3])
            direction -= 1;
        
        if(magnet[(motorPhase + 1) & 3])
            direction += 1;
    }

    // Apply motor movement
    motorPhase = MIN(70, MAX(0, (motorPhase + direction)));
    
    track = motorPhase >> 1;

    // TODO return random if addr != 0xe0
    return (addr == 0xe0) ? 0xff : 0x00;
}

void Disk::encodeAll() {
    for (int diskTrack = 0 ; diskTrack < 35 ; diskTrack ++ ) {
        encodeNibbles(diskImage->diskFile + diskAddr(diskTrack, 0), nibbles[diskTrack], true, diskTrack);
    }

    encoded = true;
}

byte Disk::readWriteData() {
    
    if(currentDrive == 0 && !writeMode) {

        // Encode (nibblize) disk data
        if(!encoded) {
            encodeAll();
        }
        
        int oldCount = byteCount;
        byteCount = (++byteCount) % NIBBLES_PER_TRACK;
        
        return nibbles[track][oldCount];
    }

    return 0;
    
}

byte Disk::selectDrive(byte drive) {
    if(DISK_LOG)
        std::cout << "Drive " << (int)drive << " selected" << std::endl;
    
    currentDrive = drive;
    return 0;
}

byte Disk::setReadMode() {
    if(DISK_LOG)
        std::cout << "Read mode" << std::endl;

    writeMode = false;
    return 0;
}

byte Disk::setWriteMode() {
    if(DISK_LOG)
        std::cout << "Write mode" << std::endl;

    writeMode = true;
    return 0;
}

byte Disk::enableDrive(bool enabled) {
    if(DISK_LOG)
        std::cout << "Drive " << (int)currentDrive << ((enabled) ? " enabled" : " disabled") << std::endl;
    
    driveOn[currentDrive] = enabled;
    return 0;
}

byte Disk::diskRead(word addr) {

    switch(addr & 0x0f) {

        case 0x0: return setPhase(0, false, addr);
        case 0x1: return setPhase(0, true, addr);
        case 0x2: return setPhase(1, false, addr);
        case 0x3: return setPhase(1, true, addr);
        case 0x4: return setPhase(2, false, addr);
        case 0x5: return setPhase(2, true, addr);
        case 0x6: return setPhase(3, false, addr);
        case 0x7: return setPhase(3, true, addr);
        case 0x8: return enableDrive(false);
        case 0x9: return enableDrive(true);
        case 0xa: return selectDrive(0);
        case 0xb: return selectDrive(1);
        case 0xc: return readWriteData();
        case 0xd: return 0; // TODO set latch value ?
        case 0xe: return setReadMode();
        case 0xf: return setWriteMode();

        default : return 0;
    }
}

void Disk::diskWrite(word addr, byte val) {

    // TODO implement writing to disk
    diskRead(addr);
}