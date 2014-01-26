#ifndef _MODULES_HPP_
#define _MODULES_HPP_

#include "internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include "internal/props.hpp"

//TODO: move elsewhere
#include "src/common/libav.hpp"
#include "src/demux/gpac_demux_mp4_simple.hpp"
#include "src/demux/gpac_demux_mp4_full.hpp"
#include "src/demux/libav_demux.hpp"
#include "src/decode/libav_decode.hpp"
#include "src/encode/libav_encode.hpp"
#include "src/in/file.hpp"
#include "src/mux/libav_mux.hpp"
#include "src/out/null.hpp"
#include "src/out/print.hpp"
#include "src/render/sdl_audio.hpp"
#include "src/render/sdl_video.hpp"
#include "src/transform/audio_convert.hpp"

#endif
