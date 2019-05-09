# Bots

Terrain parsing game for bots.

The game world is a two-dimensional orthogonal grid.
The game is turn-based. A turn ends as soon as all players have moved *or*
after one second has passed and at least one player made a move.
All actions happen in sequence.

To play the game, a bot needs to connect a streaming socket to port 63187
(or a WebSocket to port 63188) after the `bots` server has been started.

Find templates for such bots in the `templates` directory.

There is also a very useful
[WebGL visualizer](https://github.com/ChristianNorbertBraun/bots_replay)
to replay and inspect the games in a browser.

## What a bot receives from the server

At the beginning of the game and in response to each command, a bot receives
its environment as a top-down section of the whole map. This section is always
made from the same amount of columns and lines. For example, a 5x5 section
would look like this:

	.....
	....~
	.~A..
	.....
	.#...

`A` is you. You can be any letter from A to P.
First line is in front of you, the last one is behind you.

`.` is flatland, `~` is water, `#` is wood and `X` is a wall or a rock.
You can't walk through water, wood, stone or other players.

`^`, `<`, `>` and `v` is another player. The letter points in the
direction the other player is looking. For example, here, a player is
right in the front of you, looking at you:

	..v..
	....~
	.~A..
	.....
	.#...

## What a bot can send to the server

A bot _should_ respond to a map with exactly _one_ command character.

A bot _needs_ to send a command to get a new map. As long as your bot fails
to send a command, things go on without your bot knowing.

Basic commands are:

	^ - go one step forward
	< - turn left
	> - turn right
	v - go one step backward

The server will process _one_ command per turn only.

Invalid commands are silently ignored but a map is still sent.
Sending multiple command characters simply fills the network buffer and the
server will process the accumulated commands in the following turns only.

If a bot disconnects, it's immediately removed and all pending commands
are discarded.

## Available games

### training

Just learn to move around and to parse the map. Upon start, every player
gets the same section so it's easy to introduce a group of people to the
game:

	..#..
	.....
	..A..
	.....
	..~..

### escape

Find and enter the exit field `o`.
The bots are placed in a circle around this field with a random orientation.
The game ends when all players found the exit.

### collect

Collect as many gems `@` as possible.
The bots are randomly placed with a random orientation.
The bot that collected the most gems wins the game.
The game ends when there are no more gems to collect.

### snakes

Same as `collect` but every bot grows a tail for every gem it finds.
Just the like the famous game Snake. If a bot hits a tail `*`, including
its own, it's destroyed. The bot with the most gems wins the game.

### rumble

Hunt down all other bots and be the last to survive. Send `f` to shoot.
A bot can only shoot straight up and only as far as the view is.
Shots don't go through blocking cells.
The bots are placed at a regular interval.

By default, a bot is killed on the first hit but you can use `--player-life`
to change this (see below). If a bot is hit that has more than one hit point,
your name in the middle of the map, e.g. `A`, will change to a number showing
your remaining hit points.

After turn 64, a wall `X` will appear that shrinks the world to move the
remaining bots closer together (use `--shrink-after` and `--shrink-step`
to change that).

### avoid

Survive inside an asteroid shower.
All asteroids `X` move in random but diagonal directions, one field per turn.
Asteroids change direction every 10 turns.
If a bot gets hit, it's destroyed.
The bots are randomly placed with a random orientation.
The bot that survives the longest gets the most points.

### word

Find a random word of random length at the middle of the map and send it
back to the server.
The bots are placed in a circle around the word with a random orientation.
Use `--word` to set a custom word.

### boom

Place bombs and blow up all other players. Send a single digit from `1` to
`9` to place a bomb. The digit gives the number of turns until the bomb
detonates. Bombs blast orthogonal only.

Pick up power-ups `+` to increase the detonation radius of your bombs.

## Available maps

Use `--map-size` to set a custom map size and `--view-radius` to control
how many fields a bot can see of it.

Use `--map-type` to choose a map.

All four edges of a map are connected to their opposite edge.
So if a bot leaves a map on one side, it will appear on the opposite side
of the map again.

### plain

A bot can enter all fields:

	.....
	.....
	..A..
	.....
	.....

### random

A bot can _not_ enter `~` or `#`:

	#....
	....~
	.~A..
	.....
	.#...

### maze

A bot can _not_ enter `X`:

	.X.XX
	.X.X.
	.XAX.
	.X...
	XX.XX

## Build and run the server

Clone this repository:

	$ git clone https://github.com/markusfisch/bots.git

Enter the repository and the server directory and run `make`:

	$ cd bots/server && make

Then start the server:

	$ ./bots

Without any arguments you will see all modes and options:

```
usage: bots [OPTION...] MODE

MODE must be one of:
  training - just learn to see and move
  escape - find the exit field 'o'
  collect - collect as many gems '@' as possible
  snakes - eat gems '@' and grow a tail
  rumble - last man standing, shoot with 'f'
  avoid - survive an asteroid shower of 'X'
  word - find a word somewhere on the map
  boom - last man standing, place bomb with '1' to '9'

OPTION can be any of:
  -P, --port N                port to listen for players, default is 63187
  -W, --websocket-port N      port for WebSocket players, default is 63188
  -O, --spectator-port N      port for WebSocket spectators, default is 63189
  -V, --max-spectators N      maximum number of spectators, default is 0
  -r, --remote-spectators     allow remote spectators for multiplayer games
  -b, --min-starters N        minimum number of players to start a game,
                              default is 1
  -m, --min-players N         minimum number of alive players, default depends
                              on mode
  -n, --name-file FILE        list of IP addresses with player names
  -s, --map-size N[xN]        map size, default is 32x32
  -t, --map-type TYPE         map type, either "plain", "random" or "maze",
                              default is "plain"
  -c, --custom-map FILE       custom map
  -o, --obstacles STRING      characters a player cannot enter
  -f, --flatland STRING       characters a player can enter
  -x, --multiplier N          multiplier of flatland string, default is 14
  -p, --placing TYPE          player placing, either "circle", "random" or
                              "grid", default depends on mode
  -Z, --fuzzy N               maximum potential deviaton from calculated
                              position, default is 0
  -A, --place-at X,Y[,D]:...  manually place players at given coordinates and
                              in given direction, either '^', '>', 'v' or '<'
  -N, --non-exclusive         multiple players can occupy the same cell
  -v, --view-radius N         how many fields a player can see in every
                              direction, default is 2
  -G, --max-games N           maximum number of games, default is 1,
                              use -1 for unlimited games
  -M, --max-turns N           maximum number of turns, default is 1024
  -L, --max-lag N             number of turns a player can miss,
                              default is 1024
  -S, --shrink-after N        shrink map after that many turns, default is 1024
  -T, --shrink-step N         amount of turns until next shrink, default is 1
  -l, --player-life N         life value of players, default is 1
  -X, --shoot                 players can shoot, default depends on mode
  -g, --gems N                number of gems if there are gems, default equals
                              map width
  -R, --word STRING           custom word for "word" mode, random by default
  -F, --format TYPE           server output format, either "plain" or "json",
                              default is "plain"
  -w, --wait-for-joins N      number of seconds to wait for joins,
                              default is 1
  -u, --usec-per-turn N       maximum number of milliseconds per turn,
                              default is 1000000 (one second)
  -d, --deterministic         don't seed the random number generator
```

For example, to start the `escape` game, run:

	$ ./bots escape

### Requirements

Builds on everything with a decent C compiler.

For example, on a Raspberry Pi you want to install `gcc` and `make`:

	$ sudo apt-get install gcc make

## Playing manually

The `templates` directory contains a number of simple bots in different
languages. All these bots ask you for a command when a section is received.

To try the bash bot for example, type:

	$ templates/bash/bot

If the server is running on another machine, you'd do:

	$ templates/bash/bot HOSTNAME

Where HOSTNAME is either the IP address or hostname of the machine the
server is running on.

If you don't have access to a shell, you can still play the game from any
device with a web browser. Just open the file `templates/web/bot.html` or
simply click [here](https://markusfisch.github.io/bots/) and write a bot
in JavaScript.

## Playing automatically

Of course, the challenge is to write a program that plays the game.
