#include <stdio.h>
#include <string.h>

#include "../game.h"
#include "../player.h"
#include "../placing.h"
#include "find_exit.h"

#define TILE_EXIT 'O'

static int points;

static void start(struct Game *game) {
	map_set(&game->map, game->map.width / 2, game->map.height / 2, TILE_EXIT);
	placing_circle(game);
}

static void move(struct Game *game, struct Player *p, char cmd) {
	player_move(game, p, cmd);
	if (map_get(&game->map, p->x, p->y) == TILE_EXIT) {
		printf("%c escaped\n", p->name);
		p->score = points--;
		game_remove_player(game, p);
	}
}

void find_exit(struct Game *game) {
	points = 16;
	game->start = start;
	game->move = move;
	game->impassable = map_impassable;
}