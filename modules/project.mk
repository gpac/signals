MODULES_SRCS:=\
  $(ProjectName)/internal/log.cpp\
  $(ProjectName)/src/demux/gpac_mp4_simple.cpp\
  $(ProjectName)/src/demux/gpac_mp4_full.cpp\
  $(ProjectName)/src/in/file.cpp\
  $(ProjectName)/src/out/print.cpp\

MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)