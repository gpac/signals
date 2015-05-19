#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_gpacpp/gpacpp.hpp"

namespace Modules {
namespace Stream {

class MPEG_DASH : public ModuleDynI, public gpacpp::Init {
public:
	enum Type {
		Live,
		Static
	};

	MPEG_DASH(Type type, uint64_t segDurationInMs);
	~MPEG_DASH();
	void process(Data data);
	void process() override;
	void flush() override;

private:
	void DASHThread();
	void ensureInitializeDASHer();
	u32 GenerateMPD(GF_DashSegmenterInput *dasherInputs);
	void endOfStream();

	int numDataQueueNotify = 0;
	std::thread workingThread;
	Type type;
	uint64_t segDurationInMs;
	GF_Config *dashCtx;
	GF_DASHSegmenter *dasher;
};

}
}
