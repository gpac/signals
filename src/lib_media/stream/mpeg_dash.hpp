#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Stream {

struct MPD;

class MPEG_DASH : public ModuleS {
public:
	enum Type {
		Live,
		Static
	};

	MPEG_DASH(Type type, uint64_t segDurationInMs);
	~MPEG_DASH();
	void process(std::shared_ptr<const Data> data) override;
	void flush() override;

private:
	void DASHThread();
	void GenerateMPD(uint64_t segNum, std::shared_ptr<const Data> audio, std::shared_ptr<const Data> video);
	void endOfStream();

	int numDataQueueNotify = 2;
	Queue<std::shared_ptr<const Data>> audioDataQueue;
	Queue<std::shared_ptr<const Data>> videoDataQueue;
	std::thread workingThread;
	Type type;
	std::unique_ptr<MPD> mpd;
};

}
}
