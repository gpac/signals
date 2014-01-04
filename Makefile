ifeq ($(DEBUG), 1)
  CFLAGS += -g
else
  CFLAGS += -O3 -DNDEBUG
endif

all:
	cd tests && g++ -o signals $(CFLAGS) -std=c++11 -I../signals signals.cpp -lpthread

unit:
	cd tests && g++ -o signals_simple -O3 -DUNIT -std=c++11 -I../signals signals_simple.cpp
	cd tests && g++ -o signals_perf   -O3 -DUNIT -std=c++11 -I../signals signals_perf.cpp
	cd tests && g++ -o signals_module -O3 -DUNIT -std=c++11 -I../signals signals_module.cpp
	cd tests && g++ -o signals_async  -O3 -DUNIT -std=c++11 -I../signals signals_async.cpp
	
run: unit
	tests/signals_simple
	tests/signals_perf
	tests/signals_module
	tests/signals_async	
	
clean:
	#rm tests/*.o
	rm tests/signals tests/signals_simple tests/signals_perf tests/signals_module tests/signals_async