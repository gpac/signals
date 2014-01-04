all:
	cd tests && g++ -o signals -O3 -std=c++11 -I../signals signals.cpp

unit:
	cd tests && g++ -o signals_simple -O3 -DUNIT -std=c++11 -I../signals signals_simple.cpp
	cd tests && g++ -o signals_perf   -O3 -DUNIT -std=c++11 -I../signals signals_perf.cpp
	cd tests && g++ -o signals_module -O3 -DUNIT -std=c++11 -I../signals signals_module.cpp
	
run: unit
	tests/signals_simple
	tests/signals_perf
	tests/signals_module	
	
clean:
	#rm tests/*.o
	rm tests/signals tests/signals_simple tests/signals_perf tests/signals_module