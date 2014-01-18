CFLAGS := -I/usr/local/include -std=c++11 -Wall
LDFLAGS := -L/usr/local/lib -lpthread -lgpac -lavcodec -lavformat -lavutil

BIN=bin/make
SRC=.

# default to debug mode
DEBUG?=1

ifeq ($(DEBUG), 1)
  CFLAGS += -Werror -Wno-deprecated-declarations
  CFLAGS += -g3
  LDFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG -Wno-unused-variable -Wno-deprecated-declarations
endif

CFLAGS += -I$(SRC)/signals
CFLAGS += -I$(SRC)/gpacpp

ifeq ($(CXX),clang++)
  CFLAGS += -stdlib=libc++
  LDFLAGS += -stdlib=libc++
endif

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

debug:
	echo "$(CFLAGS)"

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
