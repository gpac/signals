MODULES_SRCS:=\
  $(ROOT)/internal/log.cpp\
  $(ROOT)/src/demux/gpac_mp4_simple.cpp\
  $(ROOT)/src/in/file.cpp\
  $(ROOT)/src/out/print.cpp\

MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)
