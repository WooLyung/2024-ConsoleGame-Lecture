#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <term.h>
#include <ncurses.h>
#endif

#ifndef STRUCT_GAME
#define STRUCT_GAME
struct game
{
	// functions
	void(*start)(struct game*);
	void(*finish)(struct game*);
	void(*update)(struct game*);
	
	// properties
	bool isFinished;
	int TPS;
	int width, height;

	// in-game data
	bool input1[256], input2[256];
	struct object** objects;
	int objNum, objCap;
	char* chBuffer[2];
	int* colBuffer[2];
	int curBuffer;
};
typedef struct game game;
#endif

#ifndef STRUCT_OBJECT
#define STRUCT_OBJECT
struct object
{
	int x, y;
	const char* string;
	int attributes;
};
typedef struct object object;
#endif

void launch(struct game* game);
struct game* newGame(void(*start)(struct game*), void(*finish)(struct game*), void(*update)(struct game*), int TPS, int width, int height);
struct object* newObject(int x, int y, const char* string, int attributes);
void createObject(struct game* game, struct object* object);
void deleteObject(struct game* game, struct object* object);
void setCursorVisibility(bool visible);
void setCursorPosition(int x, int y);
void setColor(int attributes);
void clearConsole();