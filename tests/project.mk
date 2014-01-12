
OUTDIR:=$(BIN)/$(ProjectName)

$(BIN)/tests/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/modules_%.o: CFLAGS+=-DUNIT

TARGETS+=$(OUTDIR)/signals_simple.exe
$(OUTDIR)/signals_simple.exe: $(OUTDIR)/signals_simple.o
DEPS+=$(OUTDIR)/signals_simple.deps

TARGETS+=$(OUTDIR)/modules_demux.exe
MODULES_DEMUX_OBJS:=$(OUTDIR)/modules_demux.o $(MODULES_OBJS)
$(OUTDIR)/modules_demux.exe: $(MODULES_DEMUX_OBJS)
DEPS+=$(MODULES_DEMUX_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/signals_perf.exe
$(OUTDIR)/signals_perf.exe: $(OUTDIR)/signals_perf.o
DEPS+=$(OUTDIR)/signals_perf.deps

TARGETS+=$(OUTDIR)/signals_module.exe
$(OUTDIR)/signals_module.exe: $(OUTDIR)/signals_module.o
DEPS+=$(OUTDIR)/signals_module.deps

TARGETS+=$(OUTDIR)/signals_async.exe
$(OUTDIR)/signals_async.exe: $(OUTDIR)/signals_async.o
DEPS+=$(OUTDIR)/signals_async.deps

TARGETS+=$(OUTDIR)/signals_unit_result.exe
$(OUTDIR)/signals_unit_result.exe: $(OUTDIR)/signals_unit_result.o
DEPS+=$(OUTDIR)/signals_unit_result.deps

TARGETS+=$(OUTDIR)/signals.exe
$(OUTDIR)/signals.exe: $(OUTDIR)/signals.o
DEPS+=$(OUTDIR)/signals.deps

TARGETS+=$(OUTDIR)/modules.exe
$(OUTDIR)/modules.exe: $(MODULES_OBJS) $(OUTDIR)/modules.o
DEPS+=$(OUTDIR)/modules.deps

run: unit
	PROGRAM=$(realpath $(OUTDIR)/modules_demux.exe) && cd $(ProjectName) && $$PROGRAM
	$(OUTDIR)/signals_unit_result.exe
	$(OUTDIR)/signals_simple.exe
	$(OUTDIR)/signals_perf.exe
	$(OUTDIR)/signals_module.exe
	$(OUTDIR)/signals_async.exe

