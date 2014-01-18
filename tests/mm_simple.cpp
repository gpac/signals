#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <memory>

using namespace Tests;
using namespace MM;

unittest("Pull2Push sub module: print packets size from file: File -> Out::Print") {
	//FIXME: this is a custom version of File
	MM::File *f = MM::File::create("data/BatmanHD_1000kbit_mpeg.mp4");
	Pull2Push *p2p = Pull2Push::create();
	std::unique_ptr<MM::Module> m(MM::Module::create(p2p, f));
	std::unique_ptr<Out::Print> p(Out::Print::create(std::cout));

	CONNECT(m.get(), signals[0]->signal, p.get(), &Out::Print::process);

	bool res = m->process();
	ASSERT(res);
}
