#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"

#include "out/print.hpp"
#include "in/file.hpp"

using namespace Tests;
using namespace MM;

unittest("File module async: File -> Out::Print") {
	auto f = uptr(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	auto p = uptr(Out::Print::create(std::cout));

	ConnectToModule(f->getPin(0)->getSignal(), p.get());

	while (f->process(nullptr)) {
	}
}

unittest("Pull2Push the File module: File -> Out::Print") {
	auto f = uptr(new MM::Pull2Push(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4")));
	auto p = uptr(Out::Print::create(std::cout));

	ConnectToModule(f->getPin(0)->getSignal(), p.get());

	bool res = f->process(nullptr); //here we call only once
	ASSERT(res);

	f->waitForCompletion();
}

unittest("Pull2Push the File module in a spawned thread: spawned(File) -> Out::Print") {
	auto f = uptr(new MM::Pull2Push(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4")));
	auto p = uptr(Out::Print::create(std::cout));

	ConnectToModule(f->getPin(0)->getSignal(), p.get());

	std::thread th(MEMBER_FUNCTOR_PROCESS(f.get()), nullptr);

	th.join();
	f->waitForCompletion();
}
