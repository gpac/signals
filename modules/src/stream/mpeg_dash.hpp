#pragma once

#include "internal/core/module.hpp"

namespace Modules {
namespace Stream {

/**
 * Open bar output. Thread-safe by design ©
 */
class MPEG_DASH : public Module {
public:
	enum Type {
		Live,
		Static
	};

	MPEG_DASH(Type type = Static);
	~MPEG_DASH();
	void process(std::shared_ptr<Data> data);
	void processAudio(std::shared_ptr<Data> data);
	void processVideo(std::shared_ptr<Data> data);

private:
	void DASHThread();
	void GenerateMPD(uint64_t segNum, std::shared_ptr<Data> audio, std::shared_ptr<Data> video);

	QueueThreadSafe<std::shared_ptr<Data>> audioDataQueue;
	QueueThreadSafe<std::shared_ptr<Data>> videoDataQueue;
	Type type;
	std::thread workingThread;
};

}
}
