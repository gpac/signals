MODULES_SRCS:=\
  $(ProjectName)/core/pipeline.cpp\
  $(ProjectName)/core/system_clock.cpp\
  $(ProjectName)/utils/stranded_pool_executor.cpp\

LIB_MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)
DEPS+=$(LIB_MODULES_OBJS:%.o=%.deps)

CFLAGS+=
