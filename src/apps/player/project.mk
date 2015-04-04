OUTDIR:=$(BIN)/$(ProjectName)

TARGETS+=$(OUTDIR)/player.exe
EXE_PLAYER_OBJS:=\
	$(LIB_MEDIA_OBJS)\
	$(LIB_MODULES_OBJS)\
	$(UTILS_OBJS)\
 	$(OUTDIR)/pipeliner.o\
 	$(OUTDIR)/player.o
$(OUTDIR)/player.exe:  $(EXE_PLAYER_OBJS)
DEPS+=$(EXE_PLAYER_OBJS:%.o=%.deps)
