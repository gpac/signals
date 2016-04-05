Signals
=======

# Introduction

Signals is a modern C++ framework to build modular applications. It is currently used for building multimedia applications. Its architecture allows to extend to any domain. Signals is used by companies from the multimedia industry (audio, video and broadcast).

Signals is designed with the following goals:
 - Adding a module must be easy. Especially for multimedia systems, you should not have to know about complex matters (types, internals, clocks, locking, ...) unless you need to ; according to our experience 90% of the applications use the same mechanisms.
 - Writing an application using modules must be easy.

# Applications

Signals comes with several multimedia applications:
 - player: a generic multimedia player.
 - dashcastx: a rewrite of the GPAC dashcast application (any input to MPEG-DASH live) in less than 300 lines of code.
 - A lot of test apps in src/tests (generators, renderers, transcoders, etc.).

# Build

Please read [build.md](doc/build.md).
 
# Tests

Signals is built using TDD. There are plenty of tests. If you contribute please commit your tests first.

# Documentation

Documentation is both a set of markdown files and a doxygen. See in the [doc subdirectory](doc/).
