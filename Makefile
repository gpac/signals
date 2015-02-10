CFLAGS:=$(CFLAGS)
CFLAGS+=-std=c++11
CFLAGS+=-Wall
CFLAGS+=-fvisibility=hidden
CFLAGS+=-W
CFLAGS+=-Wno-unused-parameter
CFLAGS+=-Wno-unused-function
CFLAGS+=-Wno-unused-label
CFLAGS+=-Wpointer-arith
CFLAGS+=-Wformat
CFLAGS+=-Wreturn-type
CFLAGS+=-Wsign-compare
CFLAGS+=-Wmultichar
CFLAGS+=-Wformat-nonliteral
CFLAGS+=-Winit-self
CFLAGS+=-Wuninitialized
CFLAGS+=-Wno-deprecated
CFLAGS+=-Wformat-security
CFLAGS+=-Wno-format-nonliteral

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

CFLAGS += -I$(SRC)/signals
CFLAGS += -I$(SRC)/gpacpp
CFLAGS += -I$(SRC)/ffpp

CFLAGS += -I$./extra/include
LDFLAGS += -L$./extra/lib

LDFLAGS += $(LDLIBS)

all: targets

$(BIN)/config.mk:
	@echo "Configuring ..."
	@set -e ; \
	export PKG_CONFIG_PATH=./extra/lib/pkgconfig:$$PKG_CONFIG_PATH ; \
	echo '# config file' > $(BIN)/config.mk.tmp ; \
	echo -n 'CFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --cflags libavcodec libavformat libswresample libswscale x264 sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs --static libavcodec libavformat libswresample libswscale x264 gpac >> $(BIN)/config.mk.tmp ; \
	sed -i "s/-lgpac/-lgpac_static/" $(BIN)/config.mk.tmp ; \
	echo 'CFLAGS+=-I./extra/include/asio' >> $(BIN)/config.mk.tmp
	echo 'LDFLAGS+=-lturbojpeg' >> $(BIN)/config.mk.tmp ; \
	mv $(BIN)/config.mk.tmp $(BIN)/config.mk ; \

include $(BIN)/config.mk

CFLAGS+=-Umain



# include sub-projects here
#

TARGETS:=
DEPS:=

ProjectName:=$(SRC)/utils
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=$(SRC)/modules
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)
CFLAGS+=-I$(ProjectName)/src

ProjectName:=$(SRC)/tests
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=$(SRC)/apps/player
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=$(SRC)/apps/dashcastx
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

$(BIN)/alldeps: $(DEPS)
	@mkdir -p "$(dir $@)"
	cat $^ > "$@"

-include $(BIN)/alldeps

