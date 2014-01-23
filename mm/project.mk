MM_SRCS:=\

MM_OBJS:=$(MM_SRCS:%.cpp=$(BIN)/%.o)

CFLAGS+=-I$(ProjectName)/internal
