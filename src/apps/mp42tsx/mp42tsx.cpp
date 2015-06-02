#include "lib_utils/profiler.hpp"
#include "pipeliner.hpp"

using namespace Pipelines;

int safeMain(int argc, char const* argv[]) {
	mp42tsXOptions opt = processArgs(argc, argv);
	
	Tools::Profiler profilerGlobal("MP42TS");

	const bool isLive = false; //FIXME: hardcoded
	Pipeline pipeline(isLive);
	declarePipeline(pipeline, opt);

	Tools::Profiler profilerProcessing("MP42TS - processing time");
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
