#include "tests.hpp"
#include <mm.hpp>
#include "../modules/modules.hpp"
#include <memory>

using namespace Tests;
using namespace MM;

unittest("Pull2Push sub module: print packets size from file: File -> Print") {
	//FIXME: this is a custom version of File
	MM::File *f = MM::File::create("data/BatmanHD_1000kbit_mpeg.mp4");
	ASSERT(f != nullptr);

	Pull2Push *p2p = Pull2Push::create();
	ASSERT(p2p != nullptr);

	std::unique_ptr<MM::Module> m(MM::Module::create(p2p, f));
	ASSERT(p2p != nullptr);

	std::unique_ptr<Print> p(Print::create(std::cout));
	ASSERT(p != nullptr);

	CONNECT(m.get(), signals[0]->signal, p.get(), &Print::process);

	bool res = m->process();
	ASSERT(res);
}
