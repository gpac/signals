<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Build](#build)
      - [Visual Studio](#visual-studio)
      - [Make](#make)
- [Run](#run)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Build

Please consult the doc/build.md file.

We support two build systems:
 - Visual Studio (tested >= 2015) (recommended on Windows)
 - Make (tested under gcc >= 4.9) (all other platforms, possible on Windows using msys2)

#### Visual Studio
For Visual Studio, dependencies are already built and available from this repository: https://github.com/rbouqueau/signals-deps.git

To use them, clone the repository and copy the pre-compiled dependencies using the CopyToSignals.bat file.
The dependencies are built using Visual Studio 2015.

#### Make
If you want to use the make build system, the dependencies for Signals need to be downloaded and built first. To do that, simply run the following script :
```
$ ./extra.sh
```
This script will ask you to install some tools (make, libtools, nasm, rsync ...). 

On Windows, to be able to use make, we recommend using [msys2](https://msys2.github.io/) which comes with the package manager pacman to install those tools. However, some environment variables including PATH need to be tweaked (especially if it contains spaces) as follows:
  64 bits:
  ```
  $ export PATH=/mingw64/bin:$PWD/extra/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw64/lib/pkgconfig
  ```
  
  32 bits:
  ```
  $ export PATH=/mingw32/bin:$PWD/extra/bin:$PATH
  $ export MSYSTEM=MINGW32
  $ export PKG_CONFIG_PATH=/mingw32/lib/pkgconfig
  ```

Once the dependencies are built, on Windows with msys2, there are still 2 problems to fix:
* The libx265 DLL is not installed correctly. Copy it from ```extra/build/src/x265/bin/x86_64-w64-mingw32/libx265.dll``` to ```extra/bin```.
* In bin/make/config.mk, remove ```-XCClinker``` and add ```-D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN``` (introduced by SDL2)

For cmake, make sure you have its subpackages installed.

Finally, you can run:
```
$ make
```

# Run
The binaries are in generated in ```bin/```including a sample player, the dashcastx application, and all the unit test apps. 

Note: For dashcastx, for live inputs with FFmpeg demux, please increase the fifo size: protocol://ip_address:port?fifo_size=1000000&overrun_nonfatal=1
