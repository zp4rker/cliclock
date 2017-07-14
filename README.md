# Cli clock

A CLI clock that displays the time in 24 or 12 hour format.

![Demo](http://i.imgur.com/pCrdAci.gif)

## Installation

    git clone https://github.com/clyde80/cliclock
    cd cliclock
    make
    sudo make install

This will install the program to `/usr/bin/cliclock`.

## Usage

You can do these while running cliclock.

Press "c" to change colors.

Press "s" to toggle the seconds field.

Press "t" to toggle 12/24 hour time format.

Or you can provide the following command line options.

-c or --color [value] -- Set the color to [value] ([0-7]).

-t or --ttime -- Use 24 hour time format.

-s or --noseconds -- Disable the seconds field.

#### Examples

`cliclock -c 1` -- Use the color Red.

`cliclock -c 1 -t` -- Use the color Red and display the time in 24 hour format (only changes foreground).

`cliclock -t` -- Display the time in 24 hour format.

`cliclock -s` -- Disable the seconds field.

`cliclock -h` -- Display the help message.

Type 'q' to exit.

Fork of the tty-clock by xorg62.
