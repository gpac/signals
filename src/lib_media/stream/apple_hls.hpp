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

	Apple_HLS(std::string const url, Type type, std::string const httpPrefix  = "", uint64_t segDurationInMs = 10000);
	~Apple_HLS();
	void process(Data data);
	void process() override;
	void flush() override;

private:
	void HLSThread();
	void endOfStream();

	std::string url;
	std::string name;
	Type type;
	uint64_t segDurationInMs;
	FILE *currentSegment;
	//Manifest manifestFile;	
	int index;
	int numDataQueueNotify = 0;
	std::thread workingThread;
};

}
}
