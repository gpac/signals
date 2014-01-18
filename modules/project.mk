MODULES_SRCS:=\
  $(ProjectName)/src/common/libav.cpp\
  $(ProjectName)/src/decode/libavcodec_55.cpp\
  $(ProjectName)/src/demux/gpac_mp4_simple.cpp\
  $(ProjectName)/src/demux/gpac_mp4_full.cpp\
  $(ProjectName)/src/demux/libavformat_55.cpp\
  $(ProjectName)/src/in/file.cpp\
  $(ProjectName)/src/out/print.cpp\

MODULES_OBJS:=$(MODULES_SRCS:%.cpp=$(BIN)/%.o)

CFLAGS+=-I$(ProjectName)/internal
