#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "player.h"
#include "modes/asteroid_shower.h"
#include "modes/collect_gems.h"
#include "modes/find_exit.h"
#include "modes/last_man_standing.h"
#include "modes/training.h"

#define MAP_TYPE_ARG_CHESS "chess"
#define MAP_TYPE_ARG_PLAIN "plain"
#define MAP_TYPE_ARG_RANDOM "random"
#define MAP_TYPE_ARG_MAZE "maze"
#define PLACING_ARG_CIRCLE "circle"
#define PLACING_ARG_RANDOM "random"

extern struct Config config;

static const char flatland[] = { TILE_FLATLAND, 0 };
static const char obstacles[] = { TILE_WATER, TILE_WOOD, 0 };
static const struct Mode {
	char *name;
	char *description;
	void *init;
} modes[] = {
	{ "training", "just learn to move and see", training },
	{ "escape", "find the exit field 'O'", find_exit },
	{ "collect", "collect as many gems '@' as possible", collect_gems },
	{ "rumble", "last man standing, shoot with 'f'", last_man_standing },
	{ "avoid", "survive inside an asteroid shower", asteroid_shower },
	{ NULL, NULL, NULL }
};

static void usage() {
	printf("usage: bots [OPTION...] MODE\n");
	printf("\nMODE must be one of:\n");
	const struct Mode *m;
	for (m = modes; m->name; ++m) {
		printf("  %s - %s\n", m->name, m->description);
	}
	printf("\nOPTION can be any of:\n"\
		"  -P, --port N            port number to listen for players\n"\
		"  -M, --min-players N     minimum number of players for a game\n"\
		"  -s, --map-size N[xN]    map size\n"\
		"  -t, --map-type TYPE     map type, either "\
			MAP_TYPE_ARG_CHESS", "\
			MAP_TYPE_ARG_PLAIN", "\
			MAP_TYPE_ARG_RANDOM" or "\
			MAP_TYPE_ARG_MAZE"\n"\
		"  -o, --obstacles STRING  characters a player cannot move over\n"\
		"  -f, --flatland STRING   characters a player can move over\n"\
		"  -x, --multiplier N      multiplier of flatland string\n"\
		"  -p, --placing TYPE      player placing, either "\
			PLACING_ARG_CIRCLE" or "\
			PLACING_ARG_RANDOM"\n"\
		"  -v, --view-radius N     how many fields a player can see in "\
			"every direction\n"\
		"  -m, --max-turns N       maximum number of turns\n"\
		"  -S, --shrink-after N    shrink map after that many turns\n"\
		"  -T, --shrink-step N     amount of turns until next shrink, "\
			"default is 1\n"\
		"  -l, --player-life N     life value of players, default is 1\n"\
		"  -g, --gems N            number of gems if there are gems\n"\
		"  -W, --wait-for-joins N  number of seconds to wait for joins\n"\
		"  -u, --usec-per-turn N   maximum number of milliseconds per turn\n"\
		"  -d, --deterministic     don't seed the random number generator\n");
}

static void *pick_mode(const char *name) {
	const struct Mode *m;
	for (m = modes; m->name; ++m) {
		if (!strcmp(m->name, name)) {
			return m->init;
		}
	}
	return NULL;
}

static int parse_placing(const char *arg) {
	if (!strcmp(arg, PLACING_ARG_CIRCLE)) {
		return PLACING_CIRCLE;
	} else if (!strcmp(arg, PLACING_ARG_RANDOM)) {
		return PLACING_RANDOM;
	}
	fprintf(stderr, "error: unknown placing \"%s\"\n", arg);
	exit(1);
}

static int parse_map_type(const char *arg) {
	if (!strcmp(arg, MAP_TYPE_ARG_CHESS)) {
		return MAP_TYPE_CHESS;
	} else if (!strcmp(arg, MAP_TYPE_ARG_PLAIN)) {
		return MAP_TYPE_PLAIN;
	} else if (!strcmp(arg, MAP_TYPE_ARG_RANDOM)) {
		return MAP_TYPE_RANDOM;
	} else if (!strcmp(arg, MAP_TYPE_ARG_MAZE)) {
		return MAP_TYPE_MAZE;
	}
	fprintf(stderr, "error: unknown map type \"%s\"\n", arg);
	exit(1);
}

static void parse_arguments(int argc, char **argv) {
	int deterministic = 0;

	struct option longopts[] = {
		{ "port", required_argument, NULL, 'P' },
		{ "min-players", required_argument, NULL, 'M' },
		{ "map-size", required_argument, NULL, 's' },
		{ "map-type", required_argument, NULL, 't' },
		{ "obstacles", required_argument, NULL, 'o' },
		{ "flatland", required_argument, NULL, 'f' },
		{ "multiplier", required_argument, NULL, 'x' },
		{ "placing", required_argument, NULL, 'p' },
		{ "view-radius", required_argument, NULL, 'v' },
		{ "max-turns", required_argument, NULL, 'm' },
		{ "shrink-after", required_argument, NULL, 'S' },
		{ "shrink-step", required_argument, NULL, 'T' },
		{ "player-life", required_argument, NULL, 'l' },
		{ "gems", required_argument, NULL, 'g' },
		{ "wait-for-joins", required_argument, NULL, 'W' },
		{ "usec-per-turn", required_argument, NULL, 'u' },
		{ "deterministic", no_argument, &deterministic, 1 },
		{ NULL, 0, NULL, 0 }
	};

	int ch;
	while ((ch = getopt_long(argc, argv, "P:M:s:t:o:f:x:p:v:m:S:T:l:g:W:u:d",
			longopts, NULL)) != -1) {
		switch (ch) {
		case 'P':
			config.port = atoi(optarg);
			break;
		case 'M':
			config.min_players = atoi(optarg);
			break;
		case 's': {
			int width = atoi(strtok(optarg, "x"));
			char *height = strtok(NULL, "x");
			config.map_width = width;
			config.map_height = height ? atoi(height) : width;
			break;
		}
		case 't':
			config.map_type = parse_map_type(optarg);
			break;
		case 'o':
			config.obstacles = optarg && *optarg ? optarg : NULL;
			break;
		case 'f':
			config.flatland = optarg && *optarg ? optarg : NULL;
			break;
		case 'x':
			config.multiplier = atoi(optarg);
			break;
		case 'p':
			config.placing = parse_placing(optarg);
			break;
		case 'v':
			config.view_radius = atoi(optarg);
			break;
		case 'm':
			config.max_turns = atoi(optarg);
			break;
		case 'S':
			config.shrink_after = atoi(optarg);
			break;
		case 'T':
			config.shrink_step = atoi(optarg);
			break;
		case 'l':
			config.player_life = atoi(optarg);
			break;
		case 'g':
			config.gems = atoi(optarg);
			break;
		case 'W':
			config.wait_for_joins = atoi(optarg);
			break;
		case 'u':
			config.usec_per_turn = atoi(optarg);
			break;
		case 'd':
			deterministic = 1;
			break;
		case 'h':
		case '?':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	void (*init_mode)() = NULL;
	if (argc < 1 || !(init_mode = pick_mode(*argv))) {
		usage();
		exit(0);
	}

	if (!deterministic) {
		srand(time(NULL));
	}

	init_mode();
}

int main(int argc, char **argv) {
	memset(&config, 0, sizeof(config));

	parse_arguments(argc, argv);

	config.port = config.port ?: 63187;
	config.min_players = config.min_players ?: 1;
	config.map_width = config.map_width ?: 32;
	config.map_height = config.map_height ?: config.map_width;
	config.map_type = config.map_type ?: MAP_TYPE_PLAIN;
	config.obstacles = config.obstacles ?: obstacles;
	config.flatland = config.flatland ?: flatland;
	config.multiplier = config.multiplier ?: 14;
	config.placing = config.placing ?: PLACING_CIRCLE;
	config.view_radius = config.view_radius ?: 2;
	config.max_turns = config.max_turns ?: 1024;
	config.shrink_after = config.shrink_after ?: config.max_turns;
	config.shrink_step = config.shrink_step ?: 1;
	config.player_life = config.player_life ?: 1;
	config.gems = config.gems ?: config.map_width;
	config.wait_for_joins = config.wait_for_joins ?: 10;
	config.usec_per_turn = config.usec_per_turn ?: USEC_PER_SEC;
	config.move = config.move ?: player_move;
	config.impassable = config.impassable ?: map_impassable;

	return game_serve();
}
