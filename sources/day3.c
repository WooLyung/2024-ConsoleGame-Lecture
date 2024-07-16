#include <stdio.h>
#include <Windows.h>
#include <conio.h>

void SetCursorVisibility(BOOL visible) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hOut, &cursorInfo);
	cursorInfo.bVisible = visible;
	SetConsoleCursorInfo(hOut, &cursorInfo);
}

void SetCursorPosition(int x, int y) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { x, y };
	SetConsoleCursorPosition(hOut, position);
}

void SetColor(WORD attributes) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut, attributes);
}

BOOL isRunning = TRUE;
int x = 0, y = 0;
int dir = 0; // 0 : 움직임X, 1234 : 상하좌우

void input() {
	while (_kbhit()) {
		unsigned char ch = _getch();
		if (ch == 0 || ch == 224) {
			int ch2 = _getch();
			if (ch2 == 72) dir = 1; // 상
			else if (ch2 == 80) dir = 2; // 하
			else if (ch2 == 75) dir = 3; // 좌
			else if (ch2 == 77) dir = 4; // 우
		}
		else {
			if (ch == 'q')
				isRunning = FALSE;
		}
	}
}

char buffer[2][10][10];
int curBuffer = 0;

void update() {
	if (dir == 1) y--;
	else if (dir == 2) y++;
	else if (dir == 3) x--;
	else if (dir == 4) x++;
}

void render() {
	if (curBuffer == 0) curBuffer = 1;
	else curBuffer = 0;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			if (i == x && j == y)
				buffer[curBuffer][i][j] = '*';
			else
				buffer[curBuffer][i][j] = ' ';

	int preBuffer = curBuffer == 1 ? 0 : 1;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			if (buffer[preBuffer][i][j] != buffer[curBuffer][i][j])
			{
				SetCursorPosition(i, j);
				SetColor(0xA);
				printf("%c", buffer[curBuffer][i][j]);
			}
}

int main() {
	SetCursorVisibility(FALSE);
	SetColor(15);
	while (isRunning)
	{
		input();
		update();
		render();
		Sleep(100);
	}
}