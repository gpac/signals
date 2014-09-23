#include "tests.hpp"
#include "modules.hpp"
#include <stdexcept>

#include "demux/gpac_demux_mp4_simple.hpp"
#include "in/file.hpp"
#include "out/print.hpp"


using namespace Tests;
using namespace Modules;

unittest("empty param test: File") {
	bool thrown = false;
	try {
		auto f = uptrSafeModule(In::File::create(""));
	} catch(std::runtime_error const& /*e*/) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Demux") {
	bool thrown = false;
	try {
		auto mp4Demux = uptrSafeModule(Demux::GPACDemuxMP4Simple::create(""));
	} catch(std::runtime_error const& /*e*/) {
		thrown = true;
	}
	ASSERT(thrown);
}

unittest("empty param test: Out::Print") {
	auto p = uptrSafeModule(new Out::Print(std::cout));
}

unittest("simple param test") {
	auto f = uptrSafeModule(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
}

unittest("print packets size from file: File -> Out::Print") {
	auto f = uptrSafeModule(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptrSafeModule(new Out::Print(std::cout));

	ConnectPinToModule(f->getPin(0), p);

	f->process(nullptr);

	f->waitForCompletion();
}

