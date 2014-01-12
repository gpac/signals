#include "tests.hpp"
#include "modules.hpp"
#include <memory>

using namespace Tests;

unittest("empty param test: File") {
	std::unique_ptr<File> f(File::create(""));
	ASSERT(f == nullptr);
}

unittest("empty param test: Demux") {
	std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create(""));
	ASSERT(mp4Demux == nullptr);
}

unittest("empty param test: Print") {
	std::unique_ptr<Print> p(Print::create());
	ASSERT(p != nullptr);
}

unittest("simple param test") {
	std::unique_ptr<File> f(File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);
}

unittest("print packets size from file: File -> Print") {
	std::unique_ptr<File> f(File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);

	std::unique_ptr<Print> p(Print::create());
	ASSERT(p != nullptr);

	CONNECT(f.get(), signals[0]->signal, p.get(), &Print::process);
	f->push();

	f->destroy();
}

