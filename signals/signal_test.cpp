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
	int numVal = sig.emit(input);
	std::vector<int> val = sig.results();
	assert(numVal == val.size());
	assert(val.size() == 1);
	assert(val[0] == dummy(input));

	val.clear();

	size_t id2 = sig.connect(dummy2);
	numVal = sig.emit(input);
	val = sig.results();
	assert(numVal == val.size());
	assert(val.size() == 2);
	assert(val[0] == dummy(input));
	assert(val[1] == dummy2(input));

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