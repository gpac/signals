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

TARGETS+=$(BIN)/tests/signals_perf.exe
$(BIN)/tests/signals_perf.exe: $(BIN)/tests/signals_perf.o

TARGETS+=$(BIN)/tests/signals_module.exe
$(BIN)/tests/signals_module.exe: $(BIN)/tests/signals_module.o

TARGETS+=$(BIN)/tests/signals_async.exe
$(BIN)/tests/signals_async.exe: $(BIN)/tests/signals_async.o

TARGETS+=$(BIN)/tests/signals_unit_result.exe
$(BIN)/tests/signals_unit_result.exe: $(BIN)/tests/signals_unit_result.o

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
	$(CXX) -c -o "$@" $^ $(CFLAGS)
	
clean:
	rm -rf $(BIN)
	mkdir $(BIN)
