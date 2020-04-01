CC=g++
CFLAGS=-mwindows -mconsole -static -static-libgcc -fpermissive
INCL=-I. -Iimgui -Iimgui/examples -Iimgui/examples/libs/gl3w
SRC=main.cpp imgui/*.cpp imgui/examples/imgui_impl_opengl3.cpp imgui/examples/imgui_impl_win32.cpp imgui/examples/libs/gl3w/GL/gl3w.c
LNK=-L'/mnt/c/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x86' -lOpenGL32 -lgdi32 -lxinput

build:
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCL) $(SRC) $(LNK) -o bin/mcpresence-gui.exe
buildr:
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCL) $(SRC) $(LNK) -O2 -s -o bin/mcpresence-gui.exe
clean:
	rm -rf bin/