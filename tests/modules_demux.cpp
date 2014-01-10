#include "tests.hpp"
#include "modules.hpp"
#include <memory>


namespace Tests {
	namespace Modules {
		namespace Demux {
			int main(int argc, char **argv) {
				Test("demux one track: GPAC_MP4_Simple -> Print");
				{
					Param paramMP4Demux;
					paramMP4Demux["filename"] = "data/BatmanHD_1000kbit_mpeg.mp4";
					std::unique_ptr<GPAC_MP4_Simple> mp4Demux(GPAC_MP4_Simple::create(paramMP4Demux));
					ASSERT(mp4Demux != nullptr);

					Param paramPrint;
					std::unique_ptr<Print> p(Print::create(paramPrint));
					ASSERT(p != nullptr);

					size_t uid = CONNECT(mp4Demux.get(), signals[0]->signal, p.get(), &Print::process);
					while (mp4Demux->process(NULL)) {
					}
				}

#if 0
				//TODO
				Test("demux two tracks: MP4Simple -> Print");

				//TODO
				Test("demux a dynamic number of tracks: MP4Simple -> Print");
#endif

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

	res = Modules::Demux::main(argc, argv);
	ASSERT(!res);

	std::cout << std::endl;
	return 0;
}
#endif

