Signals
=======

# intro

Signals is a modern C++ framework to build modular applications. It is currently used for building multimedia applications. Its architecture allows to extend to any domain. Signals is used by companies from the multimedia industry (audio, video and broadcast).

Signals is designed with the following goals:
 - Adding a module must be easy. Especially for multimedia systems, you should not have to know about complex matters (types, internals, clocks, locking, ...) unless you need to ; according to our experience 90% of the applications use the same mechanisms.
 - Writing an application using modules must be easy.

Signals include:
 - lib_signals: an agnostic signal/slot mechanism using C++11. May convey any type of data with any type of messaging.
 - lib_modules: an agnostic modules system. Uses lib_signals to connect the modules.  Modules are: inputs/outputs, a clock, an allocator, a data/metadata system. Everything can be configured thru templates. lib_modules comes with some helpers: a special thread-pool that guarantees thread-safeness of calls on a module and a pipeline class to supervise modules.
 - lib_media: a multimedia-specific layer. Uses lib_modules. Defines types for audio and video, and a lot of modules (encode/decode, mux/demux, transform, stream, render, etc.).
 - others: Signals also include some C++ wrappers for FFmpeg and GPAC, and some lib_utils (logs, profilings, C++ utils).

# applications

Signals comes with several multimedia applications:
 - player: a generic multimedia player.
 - dashcastx: a rewrite of the GPAC dashcast application (any input to MPEG-DASH live) in less than 300 lines of code.
 - A lot of test apps in src/tests.

# tests

Signals is built using TDD. There are plenty of tests. If you contribute please commit your tests first.

# build

We have two build systems:
 - Unix (tested under gcc >= 4.9)
 - Visual Studio (tested >= 2015)

## Make
 
### download/build dependencies (optional, requires ```rsync``` and ```nasm```):
```
$ ./extra.sh
```

### Build
```
$ make
```

#### MSYS2
Remove ```-XCClinker``` and add ```-D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN``` in bin/make/config.mk (introduced by SDL2)

In the Signals root path:
  
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

#### Cygwin
Same as MSYS2, plus: add ```-D__USE_W32_SOCKETS```

## Visual Studio
You need at least Visual Studio 2015.
Get the dependency repository from https://github.com/rbouqueau/signals-deps.git
Copy paste the Signals folder on the batch

# Run

For live inputs with FFmpeg demux, please increase the fifo size: protocol://ip_address:port?fifo_size=1000000&overrun_nonfatal=1
