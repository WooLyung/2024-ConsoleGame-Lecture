#include "gamelib.h"

// 출력이 되어야하는 대상
// 1. 벽
// 2. 조종하는 테트로미노
// 3. 쌓여진 테트로미노
// ----------------------
// 4. 다음 테트로미노
// 5. 킵한 테트로미노
// 6. 점수

#define WIDTH 5
#define HEIGHT 20

int tetros[7][4][2] = {
	{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, -1 } }, // 일자 블록
	{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } }, // 정사각형 블록
	{ { 0, 0 }, { -1, 0 }, { 1, 0 }, { 0, 1 } }, // ㅗ 모양
	{ { 0, 0 }, { -1, 0 }, { 1, 0 }, { 1, 1 } }, // ㄱ 모양
	{ { 0, 0 }, { -1, 0 }, { 1, 0 }, { -1, 1 } }, // ㄱ 모양
	{ { 0, 0 }, { -1, 0 }, { 0, 1 }, { 1, 1 } }, // 번개 모양
	{ { 0, 0 }, { 1, 0 }, { 0, 1 }, { -1, 1 } } // 번개 모양
};

object* curBlocks[4];
object* blocks[WIDTH][HEIGHT + 8] = { NULL }; // +x:오른쪽, +y:위쪽

int curBlockX, curBlockY; // 현재 중심 블록의 좌표
int curTetros[4][2]; // 현재 조종하고 있는 블록 파츠의 상대 좌표

int score = 0;
char scoreStr[16] = "score: 0";

void newBlock() {
	int b = rand() % 7; // 난수 생성
	curBlockX = WIDTH / 2;
	curBlockY = HEIGHT + 3;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
			curTetros[i][j] = tetros[b][i][j];
		if (b == 0)
			curBlocks[i]->attributes = 0xAA;
		else if (b == 1)
			curBlocks[i]->attributes = 0xBB;
		else if (b == 2)
			curBlocks[i]->attributes = 0xCC;
		else if (b == 3)
			curBlocks[i]->attributes = 0xDD;
		else if (b == 4)
			curBlocks[i]->attributes = 0xEE;
		else if (b == 5)
			curBlocks[i]->attributes = 0x66;
		else if (b == 6)
			curBlocks[i]->attributes = 0x99;
	}
}

void start(game* game)
{
	clearConsole();
	setCursorVisibility(false);
	setColor(0x00);

	// 점수
	createObject(game, newObject(0, HEIGHT + 2, scoreStr, 0x0F));

	// 벽
	for (int x = 0; x < WIDTH + 2; x++)
		for (int y = 0; y < HEIGHT + 2; y++)
			if (x == 0 || y == 0 || x == WIDTH + 1 || y == HEIGHT + 1)
				createObject(game, newObject(x * 2, y, "  ", 0xFF));

	// 조종하는 블록
	for (int i = 0; i < 4; i++)
	{
		curBlocks[i] = newObject(0, 0, "", 0xEE);
		createObject(game, curBlocks[i]);
	}
	newBlock();
}

int dropTime = 0;

void rotateRight() {
	bool canMove = true;
	for (int i = 0; i < 4; i++)
	{
		int x = curBlockX + curTetros[i][1];
		int y = curBlockY - curTetros[i][0];
		if (x < 0 || x >= WIDTH || y < 0 || blocks[x][y - 1])
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
		if (x < 0 || x >= WIDTH || y < 0 || blocks[x][y - 1])
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
		if (x < 0 || x >= WIDTH || y < 0 || blocks[x][y - 1])
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
		if (x < 0 || x >= WIDTH || y < 0 || blocks[x][y - 1])
			canMove = false;
	}
	if (canMove == true)
		curBlockX--;
}

bool moveDown(game* game) {
	bool canMove = true;
	for (int i = 0; i < 4; i++) {
		int x = curBlockX + curTetros[i][0];
		int y = curBlockY + curTetros[i][1] - 1;
		if (x < 0 || x >= WIDTH || y < 0 || blocks[x][y - 1])
			canMove = false;
	}
	if (canMove == true)
		curBlockY--;
	else {
		for (int i = 0; i < 4; i++) {
			int x = curBlockX + curTetros[i][0];
			int y = curBlockY + curTetros[i][1] - 1;
			blocks[x][y] = newObject(2 + x * 2, HEIGHT - y - 1, "  ", curBlocks[0]->attributes);
			createObject(game, blocks[x][y]);

			if (y >= HEIGHT)
				game->isFinished = true;
		}

		int cnt = 0;
		for (int y = 0; y < HEIGHT; y++) { // 현재 검사하는 줄
			bool pass = false;
			for (int x = 0; x < WIDTH; x++)
				if (blocks[x][y] == NULL)
					pass = true;
			if (pass == false) { // 줄 파괴
				cnt++;
				for (int x = 0; x < WIDTH; x++) // 한 줄 없애기
					deleteObject(game, blocks[x][y]);
				for (int y2 = y; y2 < HEIGHT; y2++) { // 그 위쪽 내려오기
					for (int x = 0; x < WIDTH; x++) {
						blocks[x][y2] = blocks[x][y2 + 1];
						if (blocks[x][y2])
							blocks[x][y2]->y++;
					}
				}
				y--;
			}
		}

		if (cnt != 0) {
			score += cnt;
#pragma warning(disable: 4996)
			sprintf(scoreStr, "score: %d", score);
		}

		newBlock();
	}
	return canMove;
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
		moveDown(game);
	if (game->input1['x'])
		while (moveDown(game));

	// 블록 내려오기
	dropTime++;
	if (dropTime == 60)
	{
		dropTime = 0;
		moveDown(game);
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
	setCursorVisibility(true);
	setCursorPosition(2, HEIGHT + 4);
	setColor(0x0F);
}

int main()
{
	srand(time(NULL)); // 시드값 설정
	game* game = newGame(start, finish2, update, 60, 60, 30);
	launch(game);
}