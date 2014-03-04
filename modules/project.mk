MODULES_SRCS:=\
  $(ProjectName)/internal/pin.cpp\
  $(ProjectName)/src/common/libav.cpp\
  $(ProjectName)/src/decode/libav_decode.cpp\
  $(ProjectName)/src/demux/gpac_demux_mp4_simple.cpp\
  $(ProjectName)/src/demux/gpac_demux_mp4_full.cpp\
  $(ProjectName)/src/demux/libav_demux.cpp\
  $(ProjectName)/src/encode/libav_encode.cpp\
  $(ProjectName)/src/in/file.cpp\
  $(ProjectName)/src/mux/gpac_mux_mp4.cpp\
  $(ProjectName)/src/mux/libav_mux.cpp\
  $(ProjectName)/src/out/null.cpp\
  $(ProjectName)/src/out/print.cpp\
  $(ProjectName)/src/render/sdl_audio.cpp\
  $(ProjectName)/src/render/sdl_video.cpp\
  $(ProjectName)/src/transform/audio_convert.cpp\

LIB_MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)
DEPS+=$(LIB_MODULES_OBJS:%.o=%.deps)

CFLAGS+=-I$(ProjectName)/internal
