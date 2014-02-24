OUTDIR:=$(BIN)/$(ProjectName)

TARGETS+=$(OUTDIR)/player.exe
$(OUTDIR)/player.exe: $(MODULES_OBJS) $(UTILS_OBJS) $(OUTDIR)/player.o
DEPS+=$(OUTDIR)/player.deps
