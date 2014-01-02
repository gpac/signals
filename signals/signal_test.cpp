#include "signal.hpp"

#include <iostream>


namespace {
int dummy(int a) {
	std::cout << "a = " << a << std::endl;
	return a+1;
}
int dummy2(int a) {
	return dummy(dummy(a));
}
int dummyReentrant(int a) {
	//TODO: declare a signal here?
	std::cout << "a = " << a << std::endl;
	return a + 1;
}
}

int main(int argc, char **argv) {
	Signal<int(int)> sig;

	//tests
	{
		bool res;
		res = sig.disconnect(0);
		assert(!res);
	}

	size_t id = sig.connect(dummy);
	
	const int input = 100;
	std::vector<int> val = sig.emit(input);
	assert((val.size() == 1) && (val[0] == dummy(input)));
	val.clear();

	size_t id2 = sig.connect(dummy2);
	val = sig.emit(input);
	assert((val.size() == 2) && (val[0] == dummy(input)) && (val[1] == dummy2(input)));

	//tests
	{
		bool res;
		res = sig.disconnect(id2);
		assert(res);

		res;
		res = sig.disconnect(id);
		assert(res);

		//disconnect again
		res = sig.disconnect(id);
		assert(!res);

		//wrong id
		res = sig.disconnect(id + 1);
		assert(!res);
	}

	return 0;
}