#include <iostream>
#include <stdexcept>

extern int safeMain(int argc, char const* argv[]);

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
