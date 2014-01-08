CFLAGS = -std=c++11 -Wall
LDFLAGS = -lpthread

ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG -Wno-unused-variable
endif

ifeq ($(CLANG), 1)
  CXX = clang++
else
  CXX = g++
endif

all:
	cd tests && $(CXX) -o signals $(CFLAGS) -I../signals signals.cpp $(LDFLAGS)

unit:
	cd tests && $(CXX) -o signals_unit_result -DUNIT $(CFLAGS) -I../signals signals_unit_result.cpp $(LDFLAGS)
	cd tests && $(CXX) -o signals_simple      -DUNIT $(CFLAGS) -I../signals signals_simple.cpp      $(LDFLAGS)
	cd tests && $(CXX) -o signals_perf        -DUNIT $(CFLAGS) -I../signals signals_perf.cpp        $(LDFLAGS)
	cd tests && $(CXX) -o signals_module      -DUNIT $(CFLAGS) -I../signals signals_module.cpp      $(LDFLAGS)
	cd tests && $(CXX) -o signals_async       -DUNIT $(CFLAGS) -I../signals signals_async.cpp       $(LDFLAGS)
	
run: unit
	tests/signals_unit_result
	tests/signals_simple
	tests/signals_perf
	tests/signals_module
	tests/signals_async	
	
clean:
	#rm tests/*.o
	rm tests/signals tests/signals_unit_result tests/signals_simple tests/signals_perf tests/signals_module tests/signals_async
	