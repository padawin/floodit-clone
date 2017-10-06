# Floodit

## About

### Gameplay

At each turn, select a color. The top left cell of the board will be changed to
this color along with all the adjacent cells of the same color it was before.

### Normal mode

In this mode, you have to fill the grid with a single color in less than 25
turns.

### Timed mode

Same as the Normal mode, except that a timer is running and if the game is won,
the time is stored in the high scores, for eternal fame and glory.

### Multiplayer

Following the same gameplay as the single player, each player starts in a
corner of the board and their goal is to expand their area and capture the
other players' areas.

## Screenshots

![screenshot](screenshots/main-menu.png "Main menu")
![screenshot](screenshots/in-game.png "In game")
![screenshot](screenshots/high-scores.png "High scores")

## Dependancies

Requires:

* SDL2
* SDL2_ttf
* SDL2_net
* SDL2_image

## Compilation

	make

To compile for the GCW-zero, you need the toolchain (downloadable from the
console's website), then:

	make gcw CROSS_COMPILE=/opt/gcw0-toolchain/usr/bin/mipsel-linux-

To package for the gcw zero, you will need the package squashfs-tools, then run

	make opk

Finally, to use your opk, copy it in your GCW-Zero. I use scp for that, but via
ftp or on a memory card should work as well:

	scp floodit.opk root@[gcwIp]:/media/data/apps/

## How to play

To start the game, run the executable ```floodit```.

### PC controls

* arrows to navigate in the menus and in the colors (while in game)
* space to select a menu action or to select a color
* escape to go back

### GCW-Zero controls

* D-Pad to navigate in the menus and in the colors (while in game)
* A to select a menu action or to select a color
* select to go back

## Credits

Font ClearSans from
https://01.org/clear-sans/blogs/2013/clear-sans-new-font-optimized-small-screen-readability licenced under [Apache License v2.00](https://www.fontsquirrel.com/license/clear-sans)
