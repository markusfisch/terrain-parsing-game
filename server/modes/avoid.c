#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../game.h"
#include "../player.h"
#include "avoid.h"

extern struct Config config;
extern struct Game game;

static struct Asteroid {
	int x;
	int y;
} *asteroids = NULL;
size_t nasteroids;
int vx;
int vy;
int score;

static void asteroids_move() {
	struct Asteroid *p = asteroids, *e = asteroids + nasteroids;
	for (; p < e; ++p) {
		map_set(&game.map, p->x, p->y, *config.flatland);
		p->x = map_wrap(p->x + vx, game.map.width);
		p->y = map_wrap(p->y + vy, game.map.height);
		map_set(&game.map, p->x, p->y, TILE_GONE);
		Player *hit = NULL;
		while ((hit = player_at(p->x, p->y, hit))) {
			hit->score = score++;
			game_remove_player(hit);
		}
	}
}

static void asteroids_place() {
	struct Asteroid *p = asteroids, *e = asteroids + nasteroids;
	for (; p < e; ++p) {
		do {
			p->x = rand() % game.map.width;
			p->y = rand() % game.map.height;
		} while (player_at(p->x, p->y, NULL));
	}
}

static size_t asteroids_create(int amount) {
	if (asteroids) {
		free(asteroids);
	}
	if (amount < 2) {
		amount = 2;
	}
	asteroids = calloc(amount, sizeof(struct Asteroid));
	return amount;
}

static void start() {
	nasteroids = asteroids_create(round(game.map.size * .1));
	asteroids_place();
	do {
		vx = (rand() % 3) - 1;
		vy = (rand() % 3) - 1;
	} while (!vx && !vy);
	score = MAX_PLAYERS - game.nplayers;
}

static void turn_start() {
	asteroids_move();
}

void avoid() {
	config.placing = config.placing ?: PLACING_RANDOM;
	config.view_radius = config.view_radius ?: 4;

	config.start = start;
	config.turn_start = turn_start;
}
