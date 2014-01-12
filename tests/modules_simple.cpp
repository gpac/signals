#include "tests.hpp"
#include "modules.hpp"
#include <memory>

using namespace Tests;

unittest("empty param test: File") {
	Param paramFile;
	std::unique_ptr<File> f(File::create(paramFile));
	ASSERT(f == nullptr);
}

unittest("empty param test: Demux") {
	Param paramMP4Demux;
	std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create(paramMP4Demux));
	ASSERT(mp4Demux == nullptr);
}

unittest("empty param test: Print") {
	Param paramPrint;
	std::unique_ptr<Print> p(Print::create(paramPrint));
	ASSERT(p != nullptr);
}

unittest("simple param test") {
	Param paramFile;
	paramFile["filename"] = "data/BatmanHD_1000kbit_mpeg.mp4";
	std::unique_ptr<File> f(File::create(paramFile));
	ASSERT(f != nullptr);
}

unittest("print packets size from file: File -> Print") {
	Param paramFile;
	paramFile["filename"] = "data/BatmanHD_1000kbit_mpeg.mp4";
	std::unique_ptr<File> f(File::create(paramFile));
	ASSERT(f != nullptr);

	Param paramPrint;
	std::unique_ptr<Print> p(Print::create(paramPrint));
	ASSERT(p != nullptr);

	CONNECT(f.get(), signals[0]->signal, p.get(), &Print::process);
	while (f->process(NULL)) {
	}
	f->destroy();
}

