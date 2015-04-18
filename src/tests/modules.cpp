#include "tests.hpp"
#include "lib_modules/modules.hpp"

//#define ENABLE_FAILING_TESTS

//#define R
#ifndef R
#include "modules_fifo.cpp"
#include "modules_simple.cpp"
#include "modules_clock.cpp"
#include "modules_converter.cpp"
#include "modules_decode.cpp"
#include "modules_demux.cpp"
#include "modules_encoder.cpp"
#include "modules_erasure.cpp"
#include "modules_generator.cpp"
#else
#include "modules_mux.cpp"
#endif
#ifndef R
#include "modules_player.cpp"
#include "modules_render.cpp"
#include "modules_transcoder.cpp"
#endif

using namespace Tests;
