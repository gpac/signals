#include "tests.hpp"
#include "modules.hpp"
#include <stdexcept>

#include "demux/gpac_demux_mp4_simple.hpp"
#include "in/file.hpp"
#include "out/print.hpp"

#include "../utils/tools.hpp"


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
	auto p = uptr(Out::Print::create(std::cout));
	ASSERT(p != nullptr);
}

unittest("simple param test") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);
}

unittest("print packets size from file: File -> Out::Print") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	ASSERT(f != nullptr);

	auto p = uptr(Out::Print::create(std::cout));
	ASSERT(p != nullptr);

	ConnectPinToModule(f->getPin(0), p.get());

	f->process(nullptr);

	f->waitForCompletion();
}

