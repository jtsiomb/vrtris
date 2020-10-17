vrtris - tetris in virtual reality
==================================
VRtris is a tetris game in VR. Put on your HMD, grab a cool drink (ideally with
some kind of straw), and enjoy retro puzzle gaming at its finest!

Alternatively you can build this as a regular OpenGL tetris game (gltris), by
choosing to disable VR at compile-time.

How to run in VR mode
---------------------
To run vrtris in VR, either create a configuration file called `vrtris.conf`
with the line `vr = true`, or run it with the command line argument `-vr`.

Controls
--------
While it's possible to play with the left analog stick of the oculus touch, it's
much more playable with digital inputs (d-pad on a gamepad, or keyboard).

Keyboard:
  - arrow keys or A/D/S: move left/right/down
  - up arrow, 'W', or space: rotate piece
  - enter or tab: drop
  - 'P': pause and restart after game over
  - Home: re-center in VR mode
  - F1: controls help screen

Gamepad:
  - D-pad or left analog stick: move left/right/down
  - D-pad up: drop
  - A B: rotate piece
  - start or X: pause and restart after game over
  - select: re-center in VR mode
  - Y: controls help screen

Oculus Touch (or other vendor equivalent):
  - left stick: move left/right/down
  - left stick up, and trigger buttons: drop
  - A B: rotate piece
  - X: pause and restart after game over
  - Y: controls help screen

When not running in VR mode, vrtris starts in a window by default. To take it
fullscreen hit `Alt+F11`. Alternatively you can put `fullscreen = true` in the
`vrtris.conf` file, or start it with the command line argument `-f`.

`ESC` or `Q` exits the game.

Status
------
This game is still under development. The basic gameplay works, but it's not
done yet.

License
-------
Copyright (C) 2019-2020 John Tsiombikas <nuclear@mutantstargoat.org>

This program is free software. Feel free to use, modify and/or redistribute it
under the terms of the GNU General Public License version 3, or at your option,
any later version published by the Free Software Foundation. See COPYING for
details.

Download
--------
The latest official release of vrtris is version 0.1, and it includes source
code and pre-compiled binaries for GNU/Linux and Windows. It's available as a
gzipped tarball, or a zip archive, pick whichever is more convenient, they are
exactly the same.
  - https://github.com/jtsiomb/vrtris/releases/download/v0.1/vrtris-0.1.tar.gz
  - https://github.com/jtsiomb/vrtris/releases/download/v0.1/vrtris-0.1.zip

Or if you prefer, you can grab the current source code from the git repository:

   git clone https://github.com/jtsiomb/vrtris

Dependencies
------------
VRtris depends on a number of libraries. Recursive dependencies are shown
indented, and optional dependencies are marked with *(opt)*.

  - OpenGL (any reasonably current implementation should do)
  - GLEW: http://glew.sourceforge.net
  - SDL2: http://www.libsdl.org
  - libgoatvr *(opt)*: http://github.com/jtsiomb/libgoatvr
    - gph-math: http://github.com/jtsiomb/gph-math
    - Oculus SDK *(opt)*: See below.
    - Steam OpenVR *(opt)*: https://github.com/ValveSoftware/openvr
  - libimago: http://github.com/jtsiomb/libimago
    - libpng: http://libpng.org
      - zlib: http://www.zlib.net
    - jpeglib: http://www.ijg.org
  - libdrawtext: http://github.com/jtsiomb/libdrawtext
    - freetype: http://www.freetype.org
  - libogg: http://xiph.org/ogg
  - libvorbis: http://www.xiph.org/vorbis

Note about the Oculus SDK *optional* dependency of libgoatvr:

Libgoatvr transparently supports both the current Oculus SDK (1.x) which sadly
is MS-Windows only, and the old Oculus SDK 0.5.0.1, which was the last release
for GNU/Linux systems. Unfortunately the old SDK only supports the Oculus Rift
DK1 and DK2, not the later consumer version. So those are the only Oculus HMDs
that work on GNU/Linux.

  - Oculus SDK 1.x: http://developer.oculus.com/downloads/package/oculus-sdk-for-windows
  - Oculus SDK 0.5.0.1: http://developer.oculus.com/downloads/package/oculus-sdk-for-linux

Build
-----
After building and installing all dependencies, just run `make`.

If you wish to build without VR support (which removes the dependency to
libgoatvr and all it's subdeps), change the `vrbuild` variable at the top of the
Makefile to `false`, before building.

To build on windows you need the MinGW port of GCC. Microsoft compilers are not
supported. Simply install msys2, grab the source, build and install all
dependencies, then type make.

To cross-compile for windows from UNIX, make sure you have the `i686-w64-mingw32-`
toolchain installed (debian package `mingw-w64`) and run `make cross`.
