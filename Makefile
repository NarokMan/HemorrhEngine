all:
	g++ main.cpp -o editor `pkg-config --cflags --libs sdl3` -lSDL3_image -lSDL3_ttf -lSDL3_mixer -lm
