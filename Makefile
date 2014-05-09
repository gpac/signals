CFLAGS+=-std=c++11
CFLAGS+=-Wall
CFLAGS+=-Wl,-z,relro
CFLAGS+=-Wl,-z,now
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

LDLIBS+=-lpthread
LDLIBS+=-lswscale
LDLIBS+=-lswresample
LDLIBS+=-lgpac
LDLIBS+=-lavcodec
LDLIBS+=-lavformat
LDLIBS+=-lavutil
LDLIBS+=-lz
LDLIBS+=-lSDL2
LDLIBS+=-lx264
LDLIBS+=-ldl
LDLIBS+=-liconv
LDLIBS+=-lbz2

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
CFLAGS += -I$(SRC)/modules/src
CFLAGS += -I$(SRC)/gpacpp
CFLAGS += -I$(SRC)/ffpp

CFLAGS += -I$(SRC)/extra/include
LDFLAGS += -L$(SRC)/extra/lib

ifeq ($(CXX),clang++)
  CFLAGS += -stdlib=libc++
  LDFLAGS += -stdlib=libc++
endif

LDFLAGS += $(LDLIBS)

all: targets

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

#------------------------------------------------------------------------------

targets: $(TARGETS)

unit: $(TARGETS)

include extra.mak

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
