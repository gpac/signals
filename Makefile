CFLAGS = -std=c++11 -Wall

ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG
endif

ifeq ($(CLANG), 1)
  CXX = clang++
else
  CXX = g++
endif

all:
	cd tests && $(CXX) -o signals $(CFLAGS) -I../signals signals.cpp -lpthread

unit:
	cd tests && $(CXX) -o signals_simple -DUNIT $(CFLAGS) -I../signals signals_simple.cpp
	cd tests && $(CXX) -o signals_perf   -DUNIT $(CFLAGS) -I../signals signals_perf.cpp
	cd tests && $(CXX) -o signals_module -DUNIT $(CFLAGS) -I../signals signals_module.cpp
	cd tests && $(CXX) -o signals_async  -DUNIT $(CFLAGS) -I../signals signals_async.cpp
	
run: unit
	tests/signals_simple
	tests/signals_perf
	tests/signals_module
	tests/signals_async	
	
clean:
	#rm tests/*.o
	rm tests/signals tests/signals_simple tests/signals_perf tests/signals_module tests/signals_async
	