signals
=======

Build
=====

Download dependencies (optional):
$ ./extra.sh

Build:
$ make

MSYS2:
Remove '-XCClinker' and add '-D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN' in bin/make/config.mk (introduced by SDL2)

Cygwin:
Same as MSYS2, plus: add '-D__USE_W32_SOCKETS'

In the Signals root path:
  
  64 bits:
  $ export PATH=/mingw64/bin:$PWD/extra/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw64/lib/pkgconfig
  
  32 bits:
  $ export PATH=/mingw32/bin:$PWD/extra/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw32/lib/pkgconfig

Visual Studio:
You need at least Visual Studio 2015.
Get the dependency repository from https://github.com/rbouqueau/signals-deps.git
Copy paste the Signals folder on the batch
