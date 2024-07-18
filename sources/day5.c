#include "gamelib.h"

object* apple;
object* snake[121]; // snake[0] 꼬리
int length = 3;
char score[32] = "score:3";

void start(game* game)
{
	setCursorVisibility(false);
	setColor(0x00);

	createObject(game, newObject(2, 13, score, 0x0F)); // 점수 출력
	for (int i = 0; i < 13; i++)
		for (int j = 0; j < 13; j++)
			if (i == 0 || j == 0 || i == 12 || j == 12)
				createObject(game, newObject(i * 2, j, "  ", 0xFF));

	apple = newObject(18, 6, "  ", 0xCC);
	createObject(game, apple);

	for (int i = 0; i < 3; i++)
	{
		snake[i] = newObject(4 + i * 2, 6, "  ", 0xAA);
		createObject(game, snake[i]);
	}
}

int dir = 0; // 뱀의 이동 방향
// 0 정지, 1234 : 상하좌우

void update(game* game)
{
	if (game->input1['w'] && (dir == 0 || dir == 3 || dir == 4))
		dir = 1;
	else if (game->input1['s'] && (dir == 0 || dir == 3 || dir == 4))
		dir = 2;
	else if (game->input1['a'] && (dir == 1 || dir == 2))
		dir = 3;
	else if (game->input1['d'] && (dir == 0 || dir == 1 || dir == 2))
		dir = 4;

	if (dir != 0)
	{
		int toX = snake[length - 1]->x;
		int toY = snake[length - 1]->y;
		if (dir == 1) toY--;
		else if (dir == 2) toY++;
		else if (dir == 3) toX -= 2;
		else if (dir == 4) toX += 2;

		// 벽에 닿았을 경우
		if (toX == 0 || toX == 24 || toY == 0 || toY == 12)
		{
			game->isFinished = true;
			return;
		}
		// 몸통에 닿았을 경우
		for (int i = 1; i < length; i++)
			if (snake[i]->x == toX && snake[i]->y == toY)
			{
				game->isFinished = true;
				return;
			}

		// 사과에 닿았을 경우
		if (apple->x == toX && apple->y == toY)
		{
			snake[length] = newObject(toX, toY, "  ", 0xAA);
			createObject(game, snake[length]);
			length++;

#pragma warning(disable: 4996)
			sprintf(score, "score:%d", length);

			int cnt = 0;
			int px[121], py[121];
			for (int i = 1; i < 12; i++) // x
			{
				for (int j = 1; j < 12; j++) // y
				{
					bool exist = false;
					for (int k = 0; k < length; k++) // 뱀
						if (snake[k]->x == i * 2 && snake[k]->y == j)
							exist = true;
					if (exist == false)
					{
						px[cnt] = i * 2;
						py[cnt] = j;
						cnt++;
					}
				}
			}
			srand(time(NULL));
			int r = rand() % cnt;
			apple->x = px[r];
			apple->y = py[r];
			return;
		}

		for (int i = 0; i < length - 1; i++)
		{
			snake[i]->x = snake[i + 1]->x;
			snake[i]->y = snake[i + 1]->y;
		}
		snake[length - 1]->x = toX;
		snake[length - 1]->y = toY;
	}

	if (game->input1['q'])
		game->isFinished = true;
}

void finish2(game* game)
{
	setCursorVisibility(true);
	setColor(0x0F);
	setCursorPosition(2, 15);
}

int main()
{
	game* game = newGame(start, finish2, update, 10, 40, 20); // 콘솔 크기 40 * 20
	launch(game);
}