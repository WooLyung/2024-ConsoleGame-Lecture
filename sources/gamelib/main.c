#include "gamelib.h"

object* x, *a, *b, *c, *d;

void start(game* game)
{
	clearConsole();
	setCursorVisibility(false);
	setColor(0x00);

	x = newObject(0, 0, "HELLO", 0x01);
	createObject(game, x);
	createObject(game, a = newObject(0, 1, "12", 0x02));
	createObject(game, b = newObject(0, 2, "34", 0x03));
	createObject(game, c = newObject(0, 3, "56", 0x04));
	createObject(game, d = newObject(0, 4, "78", 0x05));

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

void finish(game* game)
{
	clearConsole();
	setCursorVisibility(true);
	setColor(0x0F);
}

int main()
{
	game* game = newGame(start, finish, update, 60, 10, 30);
	launch(game);
}