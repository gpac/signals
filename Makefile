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

BIN=bin/make
SRC=.

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
CFLAGS += -I$(SRC)/mm
CFLAGS += -I$(SRC)/modules
CFLAGS += -I$(SRC)/modules/src
CFLAGS += -I$(SRC)/gpacpp
CFLAGS += -I$(SRC)/ffpp

CFLAGS += -I$(SRC)/extra/include
LDFLAGS += -L$(SRC)/extra/lib

LDFLAGS += $(LDLIBS)

all: targets

$(BIN)/config.mk:
	@echo "Configuring ..."
	@set -e ; \
	export PKG_CONFIG_PATH=$(SRC)/extra/lib/pkgconfig:$$PKG_CONFIG_PATH ; \
	echo '# config file' > $(BIN)/config.mk.tmp ; \
	echo -n 'CFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --cflags libavcodec libavformat libswresample libswscale x264 sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs sdl2 >> $(BIN)/config.mk.tmp ; \
	echo -n 'LDFLAGS+=' >> $(BIN)/config.mk.tmp ; \
	pkg-config --libs --static libavcodec libavformat libswresample libswscale x264 gpac >> $(BIN)/config.mk.tmp ; \
	sed -i "s/-lgpac/-lgpac_static/" $(BIN)/config.mk.tmp
	mv $(BIN)/config.mk.tmp $(BIN)/config.mk

include $(BIN)/config.mk

CFLAGS+=-Umain



# include sub-projects here
#

TARGETS:=
DEPS:=

ProjectName:=utils
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=modules
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=mm
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=tests
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=apps/player
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

ProjectName:=apps/dashcastx
include $(ProjectName)/project.mk
CFLAGS+=-I$(ProjectName)

#------------------------------------------------------------------------------

targets: $(TARGETS)

unit: $(TARGETS)

#------------------------------------------------------------------------------

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $^ $(LDFLAGS)
	
$(BIN)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -o "$@" $< $(CFLAGS)
	
clean:
	rm -rf $(BIN)
	mkdir $(BIN)

#-------------------------------------------------------------------------------

$(BIN)/alldeps: $(DEPS)
	@mkdir -p "$(dir $@)"
	cat $^ > "$@"

$(BIN)/%.deps: %.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CFLAGS) -c -MM "$^" -MT "$(BIN)/$*.o" > "$@"

-include $(BIN)/alldeps
