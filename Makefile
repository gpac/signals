CFLAGS = -std=c++11 -Wall
LDFLAGS = -lpthread

BIN=bin/make
SRC=.

ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG -Wno-unused-variable
endif

CFLAGS += -I$(SRC)/signals

ifeq ($(CLANG), 1)
  CXX = clang++
else
  CXX = g++
endif

all: targets

TARGETS:=

TARGETS+=$(BIN)/tests/signals_simple.exe
$(BIN)/tests/signals_simple.exe: $(BIN)/tests/signals_simple.o
DEPS+=$(BIN)/tests/signals_simple.deps

TARGETS+=$(BIN)/tests/signals_perf.exe
$(BIN)/tests/signals_perf.exe: $(BIN)/tests/signals_perf.o
DEPS+=$(BIN)/tests/signals_perf.deps

TARGETS+=$(BIN)/tests/signals_module.exe
$(BIN)/tests/signals_module.exe: $(BIN)/tests/signals_module.o
DEPS+=$(BIN)/tests/signals_module.deps

TARGETS+=$(BIN)/tests/signals_async.exe
$(BIN)/tests/signals_async.exe: $(BIN)/tests/signals_async.o
DEPS+=$(BIN)/tests/signals_async.deps

TARGETS+=$(BIN)/tests/signals_unit_result.exe
$(BIN)/tests/signals_unit_result.exe: $(BIN)/tests/signals_unit_result.o
DEPS+=$(BIN)/tests/signals_unit_result.deps

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
	
$(BIN)/%.o: %.cpp
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
