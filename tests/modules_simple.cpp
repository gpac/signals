#include "tests.hpp"
#include "modules.hpp"
#include <memory>
#include <stdexcept>

using namespace Tests;
using namespace Modules;

unittest("empty param test: File") {
	bool thrown = false;
	try {
		std::unique_ptr<In::File> f(In::File::create(""));
	}
	catch(std::runtime_error const& e) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Demux") {
	bool thrown = false;
	try {
		std::unique_ptr<Demux::GPAC_MP4_Simple> mp4Demux(Demux::GPAC_MP4_Simple::create(""));
	}
	catch(std::runtime_error const& e) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Out::Print") {
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));
	ASSERT(p != nullptr);
}

unittest("simple param test") {
	std::unique_ptr<In::File> f(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);
}

unittest("print packets size from file: File -> Out::Print") {
	std::unique_ptr<In::File> f(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);

	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));
	ASSERT(p != nullptr);

	CONNECT(f.get(), signals[0]->signal, p.get(), &Out::Print::process);
	f->push();

	f->destroy();
}

