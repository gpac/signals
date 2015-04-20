#include "tests.hpp"
#include "lib_modules/modules.hpp"

//#define ENABLE_FAILING_TESTS

//#define R
//#define P
#ifndef R
#ifndef P
#include "modules_fifo.cpp"
#include "modules_simple.cpp"
#include "modules_clock.cpp"
#include "modules_converter.cpp"
#include "modules_decode.cpp"
#include "modules_demux.cpp"
#include "modules_encoder.cpp"
#include "modules_erasure.cpp"
#include "modules_generator.cpp"
#endif
#endif
#include "modules_mux.cpp"
#ifndef R
#include "modules_pipeline.cpp"
#ifndef P
#include "modules_player.cpp"
#include "modules_render.cpp"
#include "modules_transcoder.cpp"
#endif
#endif

using namespace Tests;
