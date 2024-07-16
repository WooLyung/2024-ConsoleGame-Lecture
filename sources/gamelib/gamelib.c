#include "gamelib.h"

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
	clock_t start, end;
	double time;

#ifndef _WIN32
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	tty.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif

	game->start(game);
	while (!game->isFinished)
	{
		start = clock();
		tick(game);
		end = clock();
		time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
		if (time < 1.0 / game->TPS)
#ifdef _WIN32
			Sleep((int)((1.0 / game->TPS - time) * 1000));
#else
			usleep((int)((1.0 / game->TPS - time) * 1000000));
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