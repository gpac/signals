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

	MPEG_DASH(const std::string &mpdPath, Type type, uint64_t segDurationInMs);
	~MPEG_DASH();
	void process() override;
	void flush() override;

private:
	void DASHThread();
	void endOfStream();
	int numDataQueueNotify = 0;
	std::thread workingThread;

	void generateMPD(Type type);
	void ensureMPD();
	std::string mpdPath;
	Type type;
	uint64_t segDurationInMs, totalDurationInMs;
	std::vector<std::shared_ptr<const MetadataFile>> meta;

	std::unique_ptr<gpacpp::MPD> mpd;
};

}
}
