TARGETS+=$(BIN)/$(ProjectName)/signals_simple.exe
$(BIN)/$(ProjectName)/signals_simple.exe: $(BIN)/$(ProjectName)/signals_simple.o
DEPS+=$(BIN)/$(ProjectName)/signals_simple.deps

TARGETS+=$(BIN)/$(ProjectName)/modules_demux.exe
MODULES_DEMUX_OBJS:=$(BIN)/$(ProjectName)/modules_demux.o $(MODULES_OBJS)
$(BIN)/$(ProjectName)/modules_demux.exe: $(MODULES_DEMUX_OBJS)
DEPS+=$(MODULES_DEMUX_OBJS:%.o=%.deps)

TARGETS+=$(BIN)/$(ProjectName)/signals_perf.exe
$(BIN)/$(ProjectName)/signals_perf.exe: $(BIN)/$(ProjectName)/signals_perf.o
DEPS+=$(BIN)/$(ProjectName)/signals_perf.deps

TARGETS+=$(BIN)/$(ProjectName)/signals_module.exe
$(BIN)/$(ProjectName)/signals_module.exe: $(BIN)/$(ProjectName)/signals_module.o
DEPS+=$(BIN)/$(ProjectName)/signals_module.deps

TARGETS+=$(BIN)/$(ProjectName)/signals_async.exe
$(BIN)/$(ProjectName)/signals_async.exe: $(BIN)/$(ProjectName)/signals_async.o
DEPS+=$(BIN)/$(ProjectName)/signals_async.deps

TARGETS+=$(BIN)/$(ProjectName)/signals_unit_result.exe
$(BIN)/$(ProjectName)/signals_unit_result.exe: $(BIN)/$(ProjectName)/signals_unit_result.o
DEPS+=$(BIN)/$(ProjectName)/signals_unit_result.deps

run: unit
	$(BIN)/$(ProjectName)/modules_demux.exe
	$(BIN)/$(ProjectName)/signals_unit_result.exe
	$(BIN)/$(ProjectName)/signals_simple.exe
	$(BIN)/$(ProjectName)/signals_perf.exe
	$(BIN)/$(ProjectName)/signals_module.exe
	$(BIN)/$(ProjectName)/signals_async.exe

