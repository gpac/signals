OUTDIR:=$(BIN)/$(ProjectName)

TARGETS+=$(OUTDIR)/mp42tsx.exe
EXE_MP42TSX_OBJS:=\
	$(LIB_MEDIA_OBJS)\
	$(LIB_MODULES_OBJS)\
	$(UTILS_OBJS)\
 	$(OUTDIR)/mp42tsx.o\
 	$(OUTDIR)/options.o\
 	$(OUTDIR)/pipeliner.o
$(OUTDIR)/mp42tsx.exe:  $(EXE_MP42TSX_OBJS)
DEPS+=$(EXE_MP42TSX_OBJS:%.o=%.deps)
