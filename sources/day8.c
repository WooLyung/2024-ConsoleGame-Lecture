#include "gamelib.h"

// 1. 적 (일정 시간마다 총을 쏨, 좌우로 이동)
// 2. 플레이어 (이동, 총 쏘기)
// 3. 총알 (위/아래로 이동, 닿으면 체력 감소)

#define WIDTH 30
#define HEIGHT 15

typedef struct {
	object* obj;
	int hp, size, dir, speed, time;
} entity;

typedef struct {
	object* obj;
	int damage, speed, time;
} bullet;

entity enemies[100];
entity player;

bullet eBullets[100];
bullet pBullets[100];

void newEnemy(entity enemy) {
	for (int i = 0; i < 100; i++) {
		if (enemies[i].obj == NULL) {
			enemies[i] = enemy;
			break;
		}
	}
}

void newPlayerBullet(bullet b) {
	for (int i = 0; i < 100; i++) {
		if (pBullets[i].obj == NULL) {
			pBullets[i] = b;
			break;
		}
	}
}

void newEnemyBullet(bullet b) {
	for (int i = 0; i < 100; i++) {
		if (eBullets[i].obj == NULL) {
			eBullets[i] = b;
			break;
		}
	}
}

void start(game* game) {
	clearConsole();
	setCursorVisibility(false);
	setColor(0x00);

	// 벽 생성
	for (int y = 0; y < HEIGHT + 2; y++)
		for (int x = 0; x < WIDTH + 2; x++)
			if (x == 0 || y == 0 || x == WIDTH + 1 || y == HEIGHT + 1)
				createObject(game, newObject(x, y, " ", 0xFF));

	srand(time(NULL));
	for (int i = 0; i < 100; i++) {
		enemies[i].obj = NULL;
		pBullets[i].obj = NULL;
		eBullets[i].obj = NULL;
	}

	// 적 생성
	for (int i = 0; i < 4; i++) {
		object* obj = newObject(i * 3 + 3, 3, "('_')", 0x0C);
		createObject(game, obj);
		entity enemy = {
			obj, 5, 5, rand() % 2, rand() % 10 + 20, 0
		};
		newEnemy(enemy);
	}

	// 플레이어 생성
	object* obj = newObject(WIDTH / 2, HEIGHT - 1, "(^_^)", 0x0A);
	createObject(game, obj);
	entity p = { obj, 10, 5, 0, 0, 0 };
	player = p;
}

void update(game* game)
{
	if (game->input1['a'] && player.obj->x != 1)
		player.obj->x--;
	if (game->input1['d'] && player.obj->x + player.size - 1 != WIDTH)
		player.obj->x++;
	if (game->input1['w']) {
		object* obj = newObject(
			player.obj->x + player.size / 2, player.obj->y, "|", 0x0B);
		createObject(game, obj);
		bullet b = { obj, 2, 5, 0 };
		newPlayerBullet(b);
	}

	// 적 총알
	for (int i = 0; i < 100; i++) {
		if (eBullets[i].obj != NULL) {
			eBullets[i].time++;
			if (eBullets[i].time % eBullets[i].speed == 0) {
				eBullets[i].obj->y++;
				if (eBullets[i].obj->y < 1 || eBullets[i].obj->y > HEIGHT) {
					deleteObject(game, eBullets[i].obj);
					eBullets[i].obj = NULL;
					continue;
				}
				// 플레이어 충돌
				if (eBullets[i].obj->y == player.obj->y
					&& eBullets[i].obj->x >= player.obj->x
					&& eBullets[i].obj->x < player.obj->x + player.size) {
					// 총알 없애기
					deleteObject(game, eBullets[i].obj);
					eBullets[i].obj = NULL;
					// 데미지
					player.hp -= eBullets[i].damage;
					if (player.hp <= 0) {
						game->isFinished = true;
						break;
					}
				}
			}
		}
	}

	// 플레이어 총알
	for (int i = 0; i < 100; i++) {
		if (pBullets[i].obj != NULL) {
			pBullets[i].time++;
			if (pBullets[i].time % pBullets[i].speed == 0) {
				pBullets[i].obj->y--;
				if (pBullets[i].obj->y < 1 || pBullets[i].obj->y > HEIGHT) {
					deleteObject(game, pBullets[i].obj);
					pBullets[i].obj = NULL;
					continue;
				}
				// 적 충돌
				for (int j = 0; j < 100; j++) {
					if (enemies[j].obj != NULL) {
						if (pBullets[i].obj->y == enemies[j].obj->y
							&& pBullets[i].obj->x >= enemies[j].obj->x
							&& pBullets[i].obj->x < enemies[j].obj->x + enemies[j].size) {
							// 총알 없애기
							deleteObject(game, pBullets[i].obj);
							pBullets[i].obj = NULL;
							// 데미지
							enemies[j].hp -= pBullets[i].damage;
							if (enemies[j].hp <= 0) {
								deleteObject(game, enemies[j].obj);
								enemies[j].obj = NULL;
							}
							break;
						}
					}
				}
			}
		}
	}

	// 적
	for (int i = 0; i < 100; i++) {
		if (enemies[i].obj != NULL) {
			enemies[i].time++;
			if (enemies[i].time % enemies[i].speed == 0) {
				if (enemies[i].obj->x == 1)
					enemies[i].dir = 1;
				else if (enemies[i].obj->x + enemies[i].size - 1 == WIDTH)
					enemies[i].dir = 0;
				if (enemies[i].dir == 0)
					enemies[i].obj->x--;
				else
					enemies[i].obj->x++;

				if (rand() % 10 == 0) {
					object* obj = newObject(
						enemies[i].obj->x + enemies[i].size / 2,
						enemies[i].obj->y, "|", 0x04);
					createObject(game, obj);
					bullet b = { obj, 1, 5, 0 };
					newEnemyBullet(b);
				}
			}
		}
	}
}

void finish2(game* game)
{
	setCursorVisibility(true);
	setCursorPosition(0, 100);
	setColor(0x0F);
}

int main()
{
	game* game = newGame(start, finish2, update, 60, 60, 30);
	launch(game);
}