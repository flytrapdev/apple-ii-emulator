CC = g++

IDIR = include/
LDIR = lib/
SDIR = src

LIBS_GTK3 = -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
LIBS_SDL2 = -lSDL2 -lSDL2_image
LIBS_NFD = -lnfd

WIN_CC = x86_64-w64-mingw32-g++
WIN_LIBS_NFD = -l:nfd.lib
WIN_LIBS_SDL2 = -Lwin/sdl2_image/x86_64-w64-mingw32/lib/ -lSDL2main -Lwin/sdl2/x86_64-w64-mingw32/lib/
WIN_LIBS_GTK3 = -Lwin/gtk3/x86_64-w64-mingw32.static/lib/



WIN_IDIR_SDL2 = -Iwin/sdl2/x86_64-w64-mingw32/include/ -Iwin/sdl2_image/x86_64-w64-mingw32/include/

WIN_STATIC_FLAGS = -static-libstdc++ -static-libgcc

LIBS = $(LIBS_SDL2) $(LIBS_NFD) $(LIBS_GTK3)

CFLAGS = -Wall -I$(IDIR) -L$(LDIR)

TARGET = apple2emu
WIN_TARGET = win_apple2emu.exe

SRC = $(wildcard $(SDIR)/*.cpp)

$(TARGET):
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -o $(TARGET)

windows:
	$(WIN_CC) $(SRC) $(CFLAGS) $(WIN_STATIC_FLAGS) $(LIBS_SDL2) $(WIN_IDIR_SDL2) $(WIN_LIBS_NFD) $(WIN_LIBS_SDL2) -o $(WIN_TARGET)