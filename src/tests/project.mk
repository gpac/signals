
OUTDIR:=$(BIN)/$(ProjectName)

TEST_COMMON_OBJ:=\
	$(OUTDIR)/tests.o \
	$(OUTDIR)/utils.o
DEPS+=$(TEST_COMMON_OBJ:%.o=%.deps)

$(BIN)/$(ProjectName)/signals_%.o: CFLAGS+=-DUNIT
$(BIN)/$(ProjectName)/modules_%.o: CFLAGS+=-DUNIT
$(BIN)/$(ProjectName)/mm_%.o: CFLAGS+=-DUNIT

#---------------------------------------------------------------
# signals.exe
#---------------------------------------------------------------
SIGNALS_OBJS:=\
 	$(OUTDIR)/signals.o\
 	$(TEST_COMMON_OBJ)
DEPS+=$(SIGNALS_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/signals.exe
$(OUTDIR)/signals.exe: $(SIGNALS_OBJS)

#---------------------------------------------------------------
# modules.exe
#---------------------------------------------------------------
EXE_MODULES_OBJS:=\
 	$(OUTDIR)/modules.o\
 	$(TEST_COMMON_OBJ)\
 	$(LIB_MODULES_OBJS)\
 	$(UTILS_OBJS)
DEPS+=$(EXE_MODULES_OBJS:%.o=%.deps)

TARGETS+=$(OUTDIR)/modules.exe
$(OUTDIR)/modules.exe: $(EXE_MODULES_OBJS)

#---------------------------------------------------------------
# modules_stub.exe
#---------------------------------------------------------------
#EXE_MODULES_STUB_OBJS:=\
# 	$(OUTDIR)/modules_stub.o\
# 	$(TEST_COMMON_OBJ)\
# 	$(LIB_MODULES_OBJS)\
# 	$(UTILS_OBJS)
#DEPS+=$(EXE_MODULES_STUB_OBJS:%.o=%.deps)
#CFLAGS+=-iquote$(ProjectName)/modules_stub
#
#TARGETS+=$(OUTDIR)/modules_stub.exe
#$(OUTDIR)/modules_stub.exe: $(EXE_MODULES_STUB_OBJS)


#---------------------------------------------------------------
# run tests
#---------------------------------------------------------------

TestProjectName:=$(ProjectName)
TestOutDir:=$(OUTDIR)

run: unit
	$(TestOutDir)/modules.exe
	$(TestOutDir)/signals.exe

