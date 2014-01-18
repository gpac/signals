#include "tests.hpp"
#include "modules.hpp"
#include <memory>
#include <stdexcept>

using namespace Tests;
using namespace Modules;

unittest("empty param test: File") {
	bool thrown = false;
	try {
		std::unique_ptr<File> f(File::create(""));
	}
	catch(std::runtime_error const& e) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Demux") {
	bool thrown = false;
	try {
	std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create(""));
	}
	catch(std::runtime_error const& e) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Print") {
	std::unique_ptr<Print> p(Print::create(std::cout));
	ASSERT(p != nullptr);
}

unittest("simple param test") {
	std::unique_ptr<File> f(File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);
}

unittest("print packets size from file: File -> Print") {
	std::unique_ptr<File> f(File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);

	std::unique_ptr<Print> p(Print::create(std::cout));
	ASSERT(p != nullptr);

	CONNECT(f.get(), signals[0]->signal, p.get(), &Print::process);
	f->push();

	f->destroy();
}

