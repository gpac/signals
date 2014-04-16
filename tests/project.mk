
OUTDIR:=$(BIN)/$(ProjectName)

TEST_COMMON_OBJ:=\
	$(OUTDIR)/tests.o \
	$(OUTDIR)/utils.o
DEPS+=$(TEST_COMMON_OBJ:%.o=%.deps)

$(BIN)/tests/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/modules_%.o: CFLAGS+=-DUNIT
$(BIN)/tests/mm_%.o: CFLAGS+=-DUNIT

#---------------------------------------------------------------
# signals.exe
#---------------------------------------------------------------
SIGNALS_OBJS:=\
 	$(TEST_COMMON_OBJ)\
 	$(OUTDIR)/signals.o
DEPS+=$(SIGNALS_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/signals.exe
$(OUTDIR)/signals.exe: $(SIGNALS_OBJS)

#---------------------------------------------------------------
# modules.exe
#---------------------------------------------------------------
EXE_MODULES_OBJS:=\
 	$(TEST_COMMON_OBJ)\
 	$(LIB_MODULES_OBJS)\
 	$(UTILS_OBJS)\
 	$(OUTDIR)/modules.o
DEPS+=$(EXE_MODULES_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/modules.exe
$(OUTDIR)/modules.exe: $(EXE_MODULES_OBJS)

#---------------------------------------------------------------
# mm.exe
#---------------------------------------------------------------
TARGETS+=$(OUTDIR)/mm.exe
MM_OBJS:=\
 	$(TEST_COMMON_OBJ)\
 	$(LIB_MODULES_OBJS)\
 	$(UTILS_OBJS)\
 	$(MM_OBJS) $(OUTDIR)/mm.o
$(OUTDIR)/mm.exe: $(MM_OBJS)
DEPS+=$(OUTDIR)/mm.deps


#---------------------------------------------------------------
# run tests
#---------------------------------------------------------------

run_from = PROGRAM=$(realpath $(2)) && cd $(1) && $$PROGRAM

TestProjectName:=$(ProjectName)
TestOutDir:=$(OUTDIR)

run: unit
	$(call run_from,$(TestProjectName),$(TestOutDir)/modules.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/signals.exe)
	$(call run_from,$(TestProjectName),$(TestOutDir)/mm.exe)

