
OUTDIR:=$(BIN)/$(ProjectName)

TEST_COMMON_OBJ:=\
	$(OUTDIR)/tests.o
DEPS+=$(TEST_COMMON_OBJ:%.o=%.deps)

$(BIN)/tests/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/modules_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/mm_%.o: CFLAGS+=-DUNIT

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

TestProjectName:=$(ProjectName)
TestOutDir:=$(OUTDIR)

run: unit
	$(call run_from,$(TestProjectName),$(TestOutDir)/modules_demux.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/mm_simple.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals_unit_result.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals_simple.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals_perf.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals_module.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals_async.exe)

