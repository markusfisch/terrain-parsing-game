#include <stdlib.h>

#include "../game.h"
#include "../player.h"
#include "collect_gems.h"

extern struct Config config;
extern struct Game game;

static unsigned int collected;

void scatter_gems() {
	unsigned int i;
	for (i = 0; i < config.gems; ++i) {
		int x, y;
		do {
			x = rand() % game.map.width;
			y = rand() % game.map.height;
		} while (map_impassable(&game.map, x, y) || player_at(x, y));
		map_set(&game.map, x, y, TILE_GEM);
	}
}

static void start() {
	collected = 0;
	scatter_gems();
}

static void move(Player *p, char cmd) {
	player_move(p, cmd);
	if (map_get(&game.map, p->x, p->y) == TILE_GEM) {
		++p->score;
		map_set(&game.map, p->x, p->y, *config.flatland);
		if (++collected >= config.gems) {
			game_end();
		}
	}
}

void collect_gems() {
	config.placing = config.placing ?: PLACING_RANDOM;

	config.start = start;
	config.move = move;
}
