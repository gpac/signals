OUTDIR:=$(BIN)/$(ProjectName)

TARGETS+=$(OUTDIR)/dashcastx.exe
EXE_DASHCASTX_OBJS:=\
	$(LIB_MEDIA_OBJS)\
	$(LIB_MODULES_OBJS)\
	$(UTILS_OBJS)\
 	$(OUTDIR)/dashcastx.o
$(OUTDIR)/dashcastx.exe:  $(EXE_DASHCASTX_OBJS)
DEPS+=$(EXE_DASHASTX_OBJS:%.o=%.deps)
