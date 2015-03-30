#pragma once

#include "lib_modules/core/module.hpp"

namespace Modules {
namespace Stream {

class MPD;

/**
 * Open bar output. Thread-safe by design �
 */
class MPEG_DASH : public Module {
public:
	enum Type {
		Live,
		Static
	};

	MPEG_DASH(Type type, uint64_t segDurationInMs);
	~MPEG_DASH();
	void process(std::shared_ptr<const Data> data) override;

private:
	void DASHThread();
	void GenerateMPD(uint64_t segNum, std::shared_ptr<const Data> audio, std::shared_ptr<const Data> video);

	Queue<std::shared_ptr<const Data>> audioDataQueue;
	Queue<std::shared_ptr<const Data>> videoDataQueue;
	std::thread workingThread;
	Type type;
	std::unique_ptr<MPD> mpd;
};

}
}
