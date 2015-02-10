#include "tests.hpp"
#include "modules.hpp"

#include "lib_media/render/sdl_audio.hpp"
#include "tools.hpp"


using namespace Tests;
using namespace Modules;

unittest("Fifo") {
	GenericFifo<char> fp;
	fp.write("Hello", 5);
	fp.write("World", 5);
	ASSERT(fp.bytesToRead() == 10);

	fp.consume(4);
	ASSERT(fp.readPointer()[0] == 'o');
	ASSERT(fp.readPointer()[1] == 'W');
	ASSERT(fp.readPointer()[2] == 'o');
	ASSERT(fp.readPointer()[3] == 'r');

	fp.consume(6);
	ASSERT(fp.bytesToRead() == 0);
}
