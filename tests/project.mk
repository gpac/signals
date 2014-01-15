
OUTDIR:=$(BIN)/$(ProjectName)

TEST_COMMON_OBJ:=\
	$(OUTDIR)/tests.o
DEPS+=$(TEST_COMMON_OBJ:%.o=%.deps)

$(BIN)/tests/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/modules_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/mm_%.o: CFLAGS+=-DUNIT

TARGETS+=$(OUTDIR)/signals_simple.exe
$(OUTDIR)/signals_simple.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals_simple.o
DEPS+=$(OUTDIR)/signals_simple.deps

TARGETS+=$(OUTDIR)/modules_demux.exe
MODULES_DEMUX_OBJS:=$(TEST_COMMON_OBJ) $(OUTDIR)/modules_demux.o $(MODULES_OBJS) $(UTILS_OBJS) 
$(OUTDIR)/modules_demux.exe: $(MODULES_DEMUX_OBJS)
DEPS+=$(MODULES_DEMUX_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/signals_perf.exe
$(OUTDIR)/signals_perf.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals_perf.o
DEPS+=$(OUTDIR)/signals_perf.deps

TARGETS+=$(OUTDIR)/signals_module.exe
$(OUTDIR)/signals_module.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals_module.o
DEPS+=$(OUTDIR)/signals_module.deps

TARGETS+=$(OUTDIR)/signals_async.exe
$(OUTDIR)/signals_async.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals_async.o
DEPS+=$(OUTDIR)/signals_async.deps

TARGETS+=$(OUTDIR)/signals_unit_result.exe
$(OUTDIR)/signals_unit_result.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals_unit_result.o
DEPS+=$(OUTDIR)/signals_unit_result.deps

TARGETS+=$(OUTDIR)/mm_simple.exe
$(OUTDIR)/mm_simple.exe: $(TEST_COMMON_OBJ) $(MODULES_OBJS) $(UTILS_OBJS) $(MM_OBJS) $(OUTDIR)/mm_simple.o
DEPS+=$(OUTDIR)/mm_simple.deps

TARGETS+=$(OUTDIR)/signals.exe
$(OUTDIR)/signals.exe: $(TEST_COMMON_OBJ) $(OUTDIR)/signals.o
DEPS+=$(OUTDIR)/signals.deps

TARGETS+=$(OUTDIR)/modules.exe
$(OUTDIR)/modules.exe: $(TEST_COMMON_OBJ) $(MODULES_OBJS) $(UTILS_OBJS) $(OUTDIR)/modules.o
DEPS+=$(OUTDIR)/modules.deps

TARGETS+=$(OUTDIR)/mm.exe
$(OUTDIR)/mm.exe: $(TEST_COMMON_OBJ) $(MODULES_OBJS) $(UTILS_OBJS) $(MM_OBJS) $(OUTDIR)/mm.o
DEPS+=$(OUTDIR)/mm.deps

run_from = PROGRAM=$(realpath $(2)) && cd $(1) && $$PROGRAM

run: unit
	$(call run_from,$(ProjectName), $(OUTDIR)/modules_demux.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/mm_simple.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/signals_unit_result.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/signals_simple.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/signals_perf.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/signals_module.exe)
	$(call run_from,$(ProjectName), $(OUTDIR)/signals_async.exe)
