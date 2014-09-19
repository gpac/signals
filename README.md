signals
=======

Build
=====

Download dependencies (optional):
$ ./extra.sh

Build:
$ make

MSYS2:
  remove '-XCClinker' in bin/make/config.mk (introduced by SDL2)

  64 bits:
  $ export PATH=/mingw64/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw64/lib/pkgconfig
  
  32 bits:
  $ export PATH=/mingw32/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw32/lib/pkgconfig

Visual Studio:
You need at least Visual Studio 2014 CTP3.
Get the dependency repository from https://github.com/rbouqueau/signals-deps.git
Copy paste the Signals folder on the batch
