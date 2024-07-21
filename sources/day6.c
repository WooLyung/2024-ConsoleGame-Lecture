#include "gamelib.h"

// 출력이 되어야하는 대상
// 1. 벽
// 2. 조종하는 테트로미노
// 3. 쌓여진 테트로미노
// ----------------------
// 4. 다음 테트로미노
// 5. 킵한 테트로미노
// 6. 점수

#define WIDTH 10
#define HEIGHT 20

int tetros[7][4][2] = {
	{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, -1 } }, // 일자 블록
	{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } } // 정사각형 블록
};

object* curBlocks[4];
object* blocks[WIDTH][HEIGHT + 4]; // +x:오른쪽, +y:위쪽

int curBlockX, curBlockY; // 현재 중심 블록의 좌표
int curTetros[4][2]; // 현재 조종하고 있는 블록 파츠의 상대 좌표

void start(game* game)
{
	clearConsole();
	setCursorVisibility(false);
	setColor(0x00);

	// 벽
	for (int x = 0; x < WIDTH + 2; x++)
		for (int y = 0; y < HEIGHT + 2; y++)
			if (x == 0 || y == 0 || x == WIDTH + 1 || y == HEIGHT + 1)
				createObject(game, newObject(x * 2, y, "  ", 0xFF));

	// 조종하는 블록
	int b = 0;
	curBlockX = WIDTH / 2;
	curBlockY = HEIGHT + 3;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++)
			curTetros[i][j] = tetros[b][i][j];
	for (int i = 0; i < 4; i++)
	{
		curBlocks[i] = newObject(0, 0, "", 0xEE);
		createObject(game, curBlocks[i]);
	}
}

int dropTime = 0;

void rotateRight() {
	bool canMove = true;
	for (int i = 0; i < 4; i++)
	{
		int x = curBlockX + curTetros[i][1];
		int y = curBlockY - curTetros[i][0];
		if (x < 0 || x >= WIDTH)
			canMove = false;
	}
	if (canMove == true)
	{
		for (int i = 0; i < 4; i++)
		{
			int x = curTetros[i][1];
			int y = -curTetros[i][0];
			curTetros[i][0] = x;
			curTetros[i][1] = y;
		}
	}
}

void rotateLeft() {
	bool canMove = true;
	for (int i = 0; i < 4; i++)
	{
		int x = curBlockX - curTetros[i][1];
		int y = curBlockY + curTetros[i][0];
		if (x < 0 || x >= WIDTH)
			canMove = false;
	}
	if (canMove == true)
	{
		for (int i = 0; i < 4; i++)
		{
			int x = -curTetros[i][1];
			int y = curTetros[i][0];
			curTetros[i][0] = x;
			curTetros[i][1] = y;
		}
	}
}

void moveRight() {
	bool canMove = true;
	for (int i = 0; i < 4; i++)
	{
		int x = curBlockX + curTetros[i][0] + 1;
		int y = curBlockY + curTetros[i][1];
		if (x < 0 || x >= WIDTH)
			canMove = false;
	}
	if (canMove == true)
		curBlockX++;
}

void moveLeft() {
	bool canMove = true;
	for (int i = 0; i < 4; i++)
	{
		int x = curBlockX + curTetros[i][0] - 1;
		int y = curBlockY + curTetros[i][1];
		if (x < 0 || x >= WIDTH)
			canMove = false;
	}
	if (canMove == true)
		curBlockX--;
}

bool moveDown() {
	curBlockY--;
	return true;
}

void update(game* game)
{
	// 입력
	if (game->input1['q'])
		rotateLeft();
	if (game->input1['e'])
		rotateRight();
	if (game->input1['a'])
		moveLeft();
	if (game->input1['d'])
		moveRight();
	if (game->input1['s'])
		moveDown();

	// 블록 내려오기
	dropTime++;
	if (dropTime == 60)
	{
		dropTime = 0;
		moveDown();
	}

	for (int i = 0; i < 4; i++)
	{
		curBlocks[i]->x = 2 + (curBlockX + curTetros[i][0]) * 2;
		if (curBlockY + curTetros[i][1] < HEIGHT)
		{
			curBlocks[i]->y = HEIGHT - curBlockY - curTetros[i][1]; // curTetros[i][1]을 -로
			curBlocks[i]->string = "  ";
		}
		else
		{
			curBlocks[i]->y = 0;
			curBlocks[i]->string = "";
		}
	}
}

void finish2(game* game)
{
	clearConsole();
	setCursorVisibility(true);
	setColor(0x0F);
}

int main()
{
	game* game = newGame(start, finish2, update, 60, 60, 30);
	launch(game);
}