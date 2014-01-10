#include "tests.hpp"
#include "modules.hpp"
#include <memory>


namespace Tests {
	namespace Modules {
		namespace Simple {
			int main(int argc, char **argv) {
				Test("empty param test");
				{
					Param paramFile;
					std::unique_ptr<File> f(File::create(paramFile));
					ASSERT(f == nullptr);
				}
				{
					Param paramMP4Demux;
					std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create(paramMP4Demux));
					ASSERT(mp4Demux == nullptr);
				}
				{
					Param paramPrint;
					std::unique_ptr<Print> p(Print::create(paramPrint));
					ASSERT(p != nullptr);
				}

				Test("simple param test");
				{
					Param paramFile;
					paramFile["filename"] = "data/BatmanHD_1000kbit_mpeg.mp4";
					std::unique_ptr<File> f(File::create(paramFile));
					ASSERT(f != nullptr);
				}

				Test("print packets size from file: File -> Print");
				{
					Param paramFile;
					paramFile["filename"] = "data/BatmanHD_1000kbit_mpeg.mp4";
					std::unique_ptr<File> f(File::create(paramFile));
					ASSERT(f != nullptr);

					Param paramPrint;
					std::unique_ptr<Print> p(Print::create(paramPrint));
					ASSERT(p != nullptr);

					size_t uid = CONNECT(f.get(), signals[0]->signal, p.get(), &Print::process);
					while (f->process(NULL)) {
					}
					//Util::sleepInMs(300);
					//f->signals[0]->signal.disconnect(uid);
				}

				return 0;
			}
		}
	}
}

#ifdef UNIT
using namespace Tests;
int main(int argc, char **argv) {
	Util::Profiler p("TESTS TOTAL TIME");

	int res = 0;

	res = Modules::Simple::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif
