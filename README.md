# Minimal Solitaire

## Introduction

This is a minimal C++ implementation of the classic Klondike solitaire.  I got tired of 
ad-infested solitaires...

The C++ file solitaire.cpp is Copyrighted 2021 by Glenn Butcher, all rights reserved.  
solitaire.cpp is licensedfor use under the terms of the GPL 3.0.

## Release/Download

See the right-hand column in the github repo for a link to the release, currently just a
Windows installer.  I may make a Linux AppImage later.  I don't intend to 
expand or really even maintain this program, so 1.0 will probably be around a long time.  If
you want it to do something different, clone the repo and have at it.

## Playing

When the program is run, it starts with an empty table, only the four suit stacks.  Press one of the
New Game buttons at the top-left to deal a new game.  You'll accumulate wins and losses as
long as you deal the same draw-type; dealing a different draw-type will reset the wins
and losses accumulators.

Losses are incremented each time you deal a new game of the same type as the previous game.

Wins are incremented each time the suit stacks are fully populated.  That means, even if you've uncovered
all the cards and know you won, you still have to move all the cards to the suit stacks to get credit.
Also, when the last card is moved to the suit stacks, the game will start a simple animation, cards 
jumping out of their stacks and falling into oblivion.  You can stop that at any time by dealing a new
game.

Note: There's still a race condition in the double-click handling that will occasionally throw an 
exception.  If this occurs, just click Ignore and go on with the game.  I may yet fix that, but I'm 
losing interest...  :)

## Supporting Asset Licensing

The playing card image set used in this program is attributable to (LICENSE file included in full):
______

MIT License

Copyright (c) 2018 Howard Yeh (https://github.com/hayeah)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
______


## Building solitaire

0. Prerequisite: wxWidgets, compiled somewhere so it can be referenced with the wx-config script.
I haven't tested with the distro -dev packages, but that might also work.

1. Clone the github repo:

    $ git clone https://github.com/butcherg/solitaire.git

2. Run autoconf in the top-level repo directory:

    $ autoconf

3. Make a build directory and cd into it:

    $ mkdir build-linux
    $ cd build-linux

4. Run configure from the repo directory, specifying the location of
wxWidgets with --with-wx-config:

    $ ../configure --with-wx-config=$(HOME)/wxWidgets-3.1.4/build-linux/wx-config

Cross-compilation can be used by specifying the cross-compiler prefix in
--host.  This example specifies the mxe win64 cross-compiler:

    $ ../configure --host=x86_64-w64-mingw32.static --with-wx-config=$(HOME)/wxWidgets-3.1.4/build-win64/wx-config

5. make the program:

    $ make
    
    

