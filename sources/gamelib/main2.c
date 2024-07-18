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

#ifndef _WIN32
int _kbhit()
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

int _getch(void)
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}
#endif

static void input(struct game* game)
{
	for (int i = 0; i < 256; i++)
		game->input1[i] = game->input2[i] = false;

	while (_kbhit())
	{
		unsigned char ch = _getch();
		if (ch == 0 || ch == 224)
		{
			int ch2 = _getch();
			game->input2[ch] = true;
		}
		else
			game->input1[ch] = true;
	}
}

static void render(struct game* game)
{
	if (game->curBuffer == 0) game->curBuffer = 1;
	else game->curBuffer = 0;

	for (int x = 0; x < game->width; x++)
	{
		for (int y = 0; y < game->height; y++)
		{
			int j = y * game->width + x;
			game->chBuffer[game->curBuffer][j] = ' ';
			game->colBuffer[game->curBuffer][j] = 0x00;
		}
	}

	for (int i = 0; i < game->objNum; i++)
	{
		struct object* o = game->objects[i];
		for (int c = 0; o->string[c]; c++)
		{
			int x = o->x + c;
			int y = o->y;
			int j = y * game->width + x;
			if (j >= game->width * game->height)
				continue;
			game->chBuffer[game->curBuffer][j] = o->string[c];
			game->colBuffer[game->curBuffer][j] = o->attributes;
		}
	}

	int preBuffer = game->curBuffer == 1 ? 0 : 1;
	for (int x = 0; x < game->width; x++)
	{
		for (int y = 0; y < game->height; y++)
		{
			int j = y * game->width + x;
			if (game->chBuffer[game->curBuffer][j] != game->chBuffer[preBuffer][j]
				|| game->colBuffer[game->curBuffer][j] != game->colBuffer[preBuffer][j])
			{
				setCursorPosition(x, y);
				setColor(game->colBuffer[game->curBuffer][j]);
				printf("%c", game->chBuffer[game->curBuffer][j]);
			}
		}
	}
}

static void tick(struct game* game)
{
	input(game);
	game->update(game);
	render(game);
}

static void finish(struct game* game)
{
#ifndef _WIN32
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	tty.c_lflag |= ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif

	for (int i = 0; i < game->objNum; i++)
		free(game->objects[i]);
	free(game->objects);
	free(game->chBuffer[0]);
	free(game->chBuffer[1]);
	free(game->colBuffer[0]);
	free(game->colBuffer[1]);
	free(game);
}

void launch(struct game* game)
{
#ifndef _WIN32
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	tty.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif

	game->start(game);
	while (!game->isFinished)
	{
		tick(game);
#ifdef _WIN32
		Sleep((int)((1.0 / game->TPS) * 1000));
#else
		usleep((int)((1.0 / game->TPS) * 1000000));
#endif
	}
	game->finish(game);
	finish(game);
}

struct game* newGame(void(*start)(struct game*), void(*finish)(struct game*), void(*update)(struct game*), int TPS, int width, int height)
{
	struct game* game = (struct game*)malloc(sizeof(struct game));
	game->start = start;
	game->finish = finish;
	game->update = update;
	game->TPS = TPS;
	game->isFinished = false;
	for (int i = 0; i < 256; i++)
		game->input1[i] = game->input2[i] = false;
	game->objects = (struct object**)malloc(sizeof(struct object*) * 2);
	game->objCap = 2;
	game->objNum = 0;
	game->width = width;
	game->height = height;
	game->chBuffer[0] = (char*)malloc(sizeof(char) * width * height);
	game->chBuffer[1] = (char*)malloc(sizeof(char) * width * height);
	game->colBuffer[0] = (int*)malloc(sizeof(int) * width * height);
	game->colBuffer[1] = (int*)malloc(sizeof(int) * width * height);
	game->curBuffer = 0;
	return game;
}

void setCursorVisibility(bool visible)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hOut, &cursorInfo);
	cursorInfo.bVisible = visible;
	SetConsoleCursorInfo(hOut, &cursorInfo);
#else
	struct termios term;
	tcgetattr(STDIN_FILENO, &term);
	if (visible) {
		printf("\e[?25h");
	}
	else {
		printf("\e[?25l");
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
}

void setCursorPosition(int x, int y)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { x, y };
	SetConsoleCursorPosition(hOut, position);
#else
	int result;
	if (setupterm(NULL, STDOUT_FILENO, &result) == ERR)
	{
		return;
	}
	putp(tparm(tigetstr("cup"), y, x));
#endif
}

void setColor(int attributes)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut, attributes);
#else
	int ansi_colors[] = {
		30, // 0: Black
		34, // 1: Blue
		32, // 2: Green
		36, // 3: Cyan
		31, // 4: Red
		35, // 5: Magenta
		33, // 6: Yellow
		37, // 7: White
		90, // 8: Gray
		94, // 9: Light Blue
		92, // 10: Light Green
		96, // 11: Light Cyan
		91, // 12: Light Red
		95, // 13: Light Magenta
		93, // 14: Light Yellow
		97  // 15: Bright White
	};
	int foreground = attributes % 16;
	int background = attributes / 16;
	printf("\033[%d;%dm", ansi_colors[foreground], ansi_colors[background] + 10);
#endif
}

void createObject(struct game* game, struct object* object)
{
	if (game->objCap == game->objNum)
	{
		struct object** tmp = game->objects;
		game->objCap *= 2;
		game->objects = (struct object**)malloc(sizeof(struct object*) * game->objCap);
		for (int i = 0; i < game->objNum; i++)
			game->objects[i] = tmp[i];
		free(tmp);
	}
	game->objects[game->objNum++] = object;
}

void deleteObject(struct game* game, struct object* object)
{
	bool begin = false;
	for (int i = 0; i < game->objNum; i++)
	{
		if (begin)
			game->objects[i - 1] = game->objects[i];
		else if (game->objects[i] == object)
			begin = true;
	}

	free(object);
	game->objNum--;
	game->objects[game->objNum] = NULL;
}

struct object* newObject(int x, int y, const char* string, int attributes)
{
	struct object* object = (struct object*)malloc(sizeof(struct object));
	object->x = x;
	object->y = y;
	object->string = string;
	object->attributes = attributes;
	return object;
}

void clearConsole()
{
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

// --------------------------------------------------

object* x, * a, * b, * c, * d;

void start(game* game)
{
	clearConsole();
	setCursorVisibility(false);
	setColor(0x00);

	x = newObject(0, 0, "HELLO", 0x0F);
	createObject(game, x);
	createObject(game, a = newObject(0, 1, "12", 0x0A));
	createObject(game, b = newObject(0, 2, "34", 0x0B));
	createObject(game, c = newObject(0, 3, "56", 0x0C));
	createObject(game, d = newObject(0, 4, "78", 0x0D));

	deleteObject(game, a);
	deleteObject(game, b);
	deleteObject(game, c);
}

void update(game* game)
{
	if (game->input1['w'])
		x->y--;
	if (game->input1['s'])
		x->y++;
	if (game->input1['a'])
		x->x--;
	if (game->input1['d'])
		x->x++;
	if (game->input1['q'])
		game->isFinished = true;
}

void finish2(game* game)
{
	clearConsole();
	setCursorVisibility(true);
	setColor(0x0F);
}

int main()
{
	game* game = newGame(start, finish2, update, 60, 10, 30);
	launch(game);
}
