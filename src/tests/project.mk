OUTDIR:=$(BIN)/$(ProjectName)

TEST_COMMON_OBJ:=\
	$(OUTDIR)/tests.o
DEPS+=$(TEST_COMMON_OBJ:%.o=%.deps)

$(BIN)/$(ProjectName)/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/$(ProjectName)/modules_%.o: CFLAGS+=-DUNIT
$(BIN)/$(ProjectName)/utils_%.o: CFLAGS+=-DUNIT

#---------------------------------------------------------------
# utils.exe
#---------------------------------------------------------------
EXE_UTILS_OBJS:=\
 	$(OUTDIR)/utils.o\
 	$(TEST_COMMON_OBJ)
DEPS+=$(EXE_UTILS_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/utils.exe
$(OUTDIR)/utils.exe: $(EXE_UTILS_OBJS)

#---------------------------------------------------------------
# signals.exe
#---------------------------------------------------------------
EXE_SIGNALS_OBJS:=\
 	$(OUTDIR)/signals.o\
 	$(TEST_COMMON_OBJ)
DEPS+=$(EXE_SIGNALS_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/signals.exe
$(OUTDIR)/signals.exe: $(EXE_SIGNALS_OBJS)

#---------------------------------------------------------------
# modules.exe
#---------------------------------------------------------------
EXE_MODULES_OBJS:=\
 	$(OUTDIR)/modules.o\
 	$(TEST_COMMON_OBJ)\
 	$(LIB_MEDIA_OBJS)\
 	$(LIB_MODULES_OBJS)\
 	$(UTILS_OBJS)
DEPS+=$(EXE_MODULES_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/modules.exe
$(OUTDIR)/modules.exe: $(EXE_MODULES_OBJS)

#---------------------------------------------------------------
# run tests
#---------------------------------------------------------------

TestProjectName:=$(ProjectName)
TestOutDir:=$(OUTDIR)

run: unit
	$(TestOutDir)/utils.exe
	$(TestOutDir)/signals.exe
	cd src/tests ; ../../$(TestOutDir)/modules.exe
