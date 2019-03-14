vrtris - tetris in virtual reality
==================================

VRtris is a tetris game in VR. Put on your HMD, grab a cool drink (ideally with
some kind of straw), and enjoy retro puzzle gaming at its finest!

Alternatively you can build this as a regular OpenGL tetris game (gltris), by
choosing to disable VR at compile-time.

Status
------
This game is still under development. The basic gameplay works, but it's not
done yet.

License
-------
Copyright (C) 2019 John Tsiombikas <nuclear@mutantstargoat.org>

This program is free software. Feel free to use, modify and/or redistribute it
under the terms of the GNU General Public License version 3, or at your option,
any later version published by the Free Software Foundation. See COPYING for
details.

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

Yeah ... sorry about that. I'll provide pre-built binaries when it's ready.

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

To cross-compile for windows, make sure you have the `i686-w64-mingw32-`
toolchain installed (debian package `mingw-w64`) and run `make cross`.
