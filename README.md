# Cli clock

A CLI clock that displays the time in 24 or 12 hour time format (with seconds).

## Installation

    git clone https://github.com/clyde80/cliclock
    cd cliclock
    make
    sudo make install

This will install the program to `/usr/bin/cliclock`

## Usage

-c or --color [value] -- Set the color to [value] ([0-7]).

-t or --ttime -- Use 24 hour time format.

#### Examples

`cliclock -c 1` -- Use the color Red.

`cliclock -c 1 -t` -- Use the color Red and display the time in 24 hour format (only changes foreground).

`cliclock -t` -- Display the time in 24 hour format.

`cliclock -h` -- Display the help message.

Type 'q' to exit.

## Screenshots

*Note: Colors will vary depending on your terminal configuration.*

#### Default 
![Screenshot](http://i.imgur.com/C5l3ota.png)

#### Colors
![Screenshot](http://i.imgur.com/qUGzoc4.png)

#### 24-Hour and Colors
![Screenshot](http://i.imgur.com/TD5VrFK.png)

Fork of the tty-clock by xorg62.