all:
	cd tests && g++ -o signals -O3 -std=c++11 -I../signals signals.cpp

clean:
	#rm tests/*.o
	rm tests/signals