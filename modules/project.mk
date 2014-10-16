MODULES_SRCS:=\
  $(ProjectName)/internal/core/pin.cpp\
  $(ProjectName)/internal/core/system_clock.cpp\
  $(ProjectName)/src/common/libav.cpp\
  $(ProjectName)/src/decode/jpegturbo_decode.cpp\
  $(ProjectName)/src/decode/libav_decode.cpp\
  $(ProjectName)/src/demux/gpac_demux_mp4_simple.cpp\
  $(ProjectName)/src/demux/gpac_demux_mp4_full.cpp\
  $(ProjectName)/src/demux/libav_demux.cpp\
  $(ProjectName)/src/encode/jpegturbo_encode.cpp\
  $(ProjectName)/src/encode/libav_encode.cpp\
  $(ProjectName)/src/in/file.cpp\
  $(ProjectName)/src/in/sound_generator.cpp\
  $(ProjectName)/src/in/video_generator.cpp\
  $(ProjectName)/src/mux/gpac_mux_mp4.cpp\
  $(ProjectName)/src/mux/libav_mux.cpp\
  $(ProjectName)/src/out/file.cpp\
  $(ProjectName)/src/out/null.cpp\
  $(ProjectName)/src/out/print.cpp\
  $(ProjectName)/src/render/sdl_audio.cpp\
  $(ProjectName)/src/render/sdl_common.cpp\
  $(ProjectName)/src/render/sdl_video.cpp\
  $(ProjectName)/src/transform/audio_convert.cpp\
  $(ProjectName)/src/transform/video_convert.cpp\
  $(ProjectName)/internal/utils/stranded_pool_executor.cpp\

LIB_MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)
DEPS+=$(LIB_MODULES_OBJS:%.o=%.deps)

CFLAGS+=
