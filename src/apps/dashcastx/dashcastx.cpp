#include "lib_utils/profiler.hpp"
#include "pipeliner.hpp"

using namespace Modules;

int safeMain(int argc, char const* argv[]) {
	dashcastXOptions opt = processArgs(argc, argv);
	
	Tools::Profiler profilerGlobal("DashcastX");

	Pipeline pipeline(opt.isLive);
	declarePipeline(pipeline, opt);

	Tools::Profiler profilerProcessing("Dashcast X - processing time");
	pipeline.start();
	pipeline.waitForCompletion();

	return 0;
}

int main(int argc, char const* argv[]) {
	try {
		return safeMain(argc, argv);
	} catch(std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
}
