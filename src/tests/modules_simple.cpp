#include "tests.hpp"
#include "lib_modules/modules.hpp"
#include <stdexcept>

#include "lib_media/demux/gpac_demux_mp4_simple.hpp"
#include "lib_media/in/file.hpp"
#include "lib_media/out/print.hpp"
#include "lib_utils/tools.hpp"


using namespace Tests;
using namespace Modules;

unittest("empty param test: File") {
	bool thrown = false;
	try {
		auto f = uptr(In::File::create(""));
	} catch(std::runtime_error const& /*e*/) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Demux") {
	bool thrown = false;
	try {
		auto mp4Demux = uptr(Demux::GPACDemuxMP4Simple::create(""));
	} catch(std::runtime_error const& /*e*/) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Out::Print") {
	auto p = uptr(new Out::Print(std::cout));
}

unittest("simple param test") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
}

unittest("print packets size from file: File -> Out::Print") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptr(new Out::Print(std::cout));

	ConnectPinToModule(f->getPin(0), p);

	f->process(nullptr);
}

