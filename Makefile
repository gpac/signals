CFLAGS:=$(CFLAGS)
CFLAGS+=-std=gnu++1y
CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-fvisibility=hidden
CFLAGS+=-W
CFLAGS+=-Wno-unused-parameter
CFLAGS+=-Wno-unused-function
CFLAGS+=-Wno-unused-label
CFLAGS+=-Wno-write-strings

CFLAGS+=-D__STDC_CONSTANT_MACROS

BIN=bin
SRC=src

# default to debug mode
DEBUG?=1

ifeq ($(DEBUG), 1)
  CFLAGS += -Werror -Wno-deprecated-declarations
  CFLAGS += -g3
  LDFLAGS += -g
else
  CFLAGS += -s -O3 -DNDEBUG -Wno-unused-variable -Wno-deprecated-declarations
  LDFLAGS += -s
endif

CFLAGS += -I$(SRC) -I$(SRC)/lib_modules

CFLAGS += -I./extra/include
LDFLAGS += -L./extra/lib

LDFLAGS += $(LDLIBS)

all: targets

$(BIN)/config.mk:
	@echo "Configuring ..."
	@set -e ; \
	mkdir -p $(BIN)
	export PKG_CONFIG_PATH=./extra/lib/pkgconfig:$$PKG_CONFIG_PATH ; \
	echo '# config file' > $(BIN)/config.mk.tmp ; \
	echo -n 'CFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --cflags libavcodec libavdevice libavformat libswresample libswscale x264 sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs --static libavcodec libavdevice libavformat libswresample libswscale x264 gpac >> $(BIN)/config.mk.tmp ; \
	sed -i "s/-lgpac/-lgpac_static/" $(BIN)/config.mk.tmp ; \
	echo 'CFLAGS+=-I./extra/include/asio' >> $(BIN)/config.mk.tmp
	echo 'LDFLAGS+=-lturbojpeg' >> $(BIN)/config.mk.tmp ; \
	mv $(BIN)/config.mk.tmp $(BIN)/config.mk ; \

include $(BIN)/config.mk

CFLAGS+=-Umain

TARGETS:=
DEPS:=

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/lib_utils
UTILS_SRCS:=\
  $(ProjectName)/log.cpp\

UTILS_OBJS:=$(UTILS_SRCS:%.cpp=$(BIN)/%.o)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/lib_media
MEDIA_SRCS:=\
  $(ProjectName)/common/libav.cpp\
  $(ProjectName)/common/picture.cpp\
  $(ProjectName)/decode/jpegturbo_decode.cpp\
  $(ProjectName)/decode/libav_decode.cpp\
  $(ProjectName)/demux/gpac_demux_mp4_simple.cpp\
  $(ProjectName)/demux/gpac_demux_mp4_full.cpp\
  $(ProjectName)/demux/libav_demux.cpp\
  $(ProjectName)/encode/jpegturbo_encode.cpp\
  $(ProjectName)/encode/libav_encode.cpp\
  $(ProjectName)/in/file.cpp\
  $(ProjectName)/in/sound_generator.cpp\
  $(ProjectName)/in/video_generator.cpp\
  $(ProjectName)/mux/gpac_mux_m2ts.cpp\
  $(ProjectName)/mux/gpac_mux_mp4.cpp\
  $(ProjectName)/mux/libav_mux.cpp\
  $(ProjectName)/out/file.cpp\
  $(ProjectName)/out/null.cpp\
  $(ProjectName)/out/print.cpp\
  $(ProjectName)/render/sdl_audio.cpp\
  $(ProjectName)/render/sdl_common.cpp\
  $(ProjectName)/render/sdl_video.cpp\
  $(ProjectName)/stream/apple_hls.cpp\
  $(ProjectName)/stream/mpeg_dash.cpp\
  $(ProjectName)/transform/audio_convert.cpp\
  $(ProjectName)/transform/restamp.cpp\
  $(ProjectName)/transform/video_convert.cpp\
  $(ProjectName)/utils/comparator.cpp\
  $(ProjectName)/utils/recorder.cpp\

LIB_MEDIA_OBJS:=$(MEDIA_SRCS:%.cpp=$(BIN)/%.o)
DEPS+=$(LIB_MEDIA_OBJS:%.o=%.deps)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/lib_modules
MODULES_SRCS:=\
  $(ProjectName)/core/system_clock.cpp\
  $(ProjectName)/utils/pipeline.cpp\
  $(ProjectName)/utils/stranded_pool_executor.cpp\

LIB_MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)
DEPS+=$(LIB_MODULES_OBJS:%.o=%.deps)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/tests
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/apps/player
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/apps/dashcastx
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

#------------------------------------------------------------------------------

ProjectName:=$(SRC)/apps/mp42tsx
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

#------------------------------------------------------------------------------

targets: $(TARGETS)

unit: $(TARGETS)

#------------------------------------------------------------------------------

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $^ $(LDFLAGS)
	
$(BIN)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) "$<" -c $(CFLAGS) -o "$(BIN)/$*.deps" -MM -MT "$(BIN)/$*.o"
	$(CXX) "$<" -c $(CFLAGS) -o "$@" 
	
clean:
	rm -rf $(BIN)
	mkdir $(BIN)

#-------------------------------------------------------------------------------

-include $(DEPS)

