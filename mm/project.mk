MM_SRCS:=\
  $(ProjectName)/src/in/file.cpp\

MM_OBJS:=$(MM_SRCS:%.cpp=$(BIN)/%.o)

CFLAGS+=-I$(ProjectName)/internal
