# Apple II emulator
A primitive Apple II emulator written in C++ using SDL2.

This program currently emulates the original Apple II, without RAM extensions.  
It can run BASIC and play games from floppy disk images.  
(Games that require more than 48k of RAM will not run)

Only DSK and DO disk images are supported.

This is a recreational project : it does not aim to be a complete, accurate Apple II emulator.  
For that purpose, I suggest looking at AppleWin and LinApple.

![karateka](https://user-images.githubusercontent.com/68333938/191476791-8ee460dc-ee88-4e92-88b4-4ea884919c48.png)

![mysteryhouse](https://user-images.githubusercontent.com/68333938/191476806-bd6e1e2c-e52b-471f-b931-a32660c6d84a.png)

## Usage

F2 : Reset emulator  
F3 : Load disk image and reboot  
F11: Toggle between color and black/white video emulation.

## Building for Linux

The emulator can be built on Linux using g++.  

The following components are required :  
- G++ compiler
- Make
- SDL2
- SDL2_image
- GTK 3
- Native File Dialog Extended

Under Ubuntu, the required libraries can be installed using this command :
```
apt install gcc make libsdl2-dev libsdl2-image-dev libgtk-3-dev
```

The emulator uses the Native File Dialog library (NFDe) for the file window.  
It is licensed under the zlib license.  
Download it from this repository and build it : [btzy/nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended)

Create a folder named ```lib``` at the root of the repository and place the NFD binary lib file inside it.

Finally, build the emulator using make :
```
make
```

## Building from Windows

Cross-compilation towards Windows is possible, but tedious due to the various libraries needed.  
Windows development binaries for SDL2, GTK3 and NFDe are required.  

Create the following folders at the root of the repository :
```
win/sdl2
win/sdl2_image
win/gtk3
```

Place the MinGW binaries inside these folders for the corresponding libraries.  

You will also need to install MinGW-w64.
On Ubuntu :
```
apt install mingw-w64
```

Build the Native File Dialog library for Windows and place the .a file in the ```lib``` folder.

Finally, use make to build the emulator :
```
make windows
```

The current Makefile is a draft, there's a good chance Windows compilation will not work on the first try.  
Please refer to the Makefile and modify it if necessary.  

## Known issues
- The emulator runs too fast unless the monitor refresh rate is set to 60 Hz. I wasn't able to get a stable 60fps otherwise with SDL2.
- Sound is not emulated.
- Keyboard emulation is incomplete, Ctrl and Alt key combinations do not work.
- Floppy disk emulation is primitive, writing to disk is not possible.

## License
The emulator is licensed under the terms of the GPLv3 license.  

This repository includes a copy of the Apple II ROM, which is still copyrighted by Apple.  
I don't think I am causing them much harm by including it, however I will remove it from the repo if Apple requests it.
