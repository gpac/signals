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
	void process(std::shared_ptr<const Data> data) override;

private:
	void DASHThread();
	void GenerateMPD(uint64_t segNum, std::shared_ptr<const Data> audio, std::shared_ptr<const Data> video);

	Queue<std::shared_ptr<const Data>> audioDataQueue;
	Queue<std::shared_ptr<const Data>> videoDataQueue;
	Type type;
	std::thread workingThread;
};

}
}
