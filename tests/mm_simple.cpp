#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <memory>

using namespace Tests;
using namespace MM;

#if 0 //FIXME: duplicate test
unittest("File module async: File -> Out::Print") {
	std::unique_ptr<In::File> f(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4"));
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(f->getPin(0)->getSignal(), p.get(), &Out::Print::process);

	while (f->process(nullptr)) {
	}
}
#endif

unittest("Pull2Push the File module: File -> Out::Print") {
	std::unique_ptr<MM::Pull2Push> f(new MM::Pull2Push(In::File::create("data/BatmanHD_1000kbit_mpeg.mp4")));
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	Connect(f->getPin(0)->getSignal(), p.get(), &Out::Print::process);

	bool res = f->process(nullptr); //here we call only once
	ASSERT(res);

	f->waitForCompletion();
}
