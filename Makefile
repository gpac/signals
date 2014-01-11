CFLAGS := -std=c++11 -Wall
LDFLAGS := -lpthread -lgpac

BIN=bin/make
SRC=.

ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG -Wno-unused-variable
endif

CFLAGS += -I$(SRC)/signals
CFLAGS += -I$(SRC)/gpacpp

ifeq ($(CLANG), 1)
  CXX = clang++
else
  CXX = g++
endif

all: targets

# include sub-projects here
#

TARGETS:=
DEPS:=

ProjectName:=modules
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

run: unit
	$(BIN)/tests/signals_unit_result.exe
	$(BIN)/tests/signals_simple.exe
	$(BIN)/tests/signals_perf.exe
	$(BIN)/tests/signals_module.exe
	$(BIN)/tests/signals_async.exe

$(BIN)/tests/%.o: CFLAGS+=-DUNIT

$(BIN)/signals.exe: $(BIN)/signals.o

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
