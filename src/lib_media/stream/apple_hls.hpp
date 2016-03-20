#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_gpacpp/gpacpp.hpp"

namespace Modules {
namespace Stream {

class Apple_HLS : public ModuleDynI {
public:
	enum Type {
		Live,
		Static
	};

	Apple_HLS(Type type, uint64_t segDurationInMs);
	~Apple_HLS();
	void process() override;
	void flush() override;

private:
	void HLSThread();
	u32 GenerateM3U8();
	void endOfStream();

	int numDataQueueNotify = 0;
	std::thread workingThread;
	Type type;
	uint64_t segDurationInMs;
};

}
}
