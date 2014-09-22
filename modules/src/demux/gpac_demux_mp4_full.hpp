#pragma once

#include "internal/core/module.hpp"
#include <cstdint>
#include <string>

using namespace Modules;

namespace Demux {

class ISOProgressiveReader;

class GPACDemuxMP4Full : public Module {
public:
	static GPACDemuxMP4Full* create();
	~GPACDemuxMP4Full();
	bool process(std::shared_ptr<Data> data);

private:
	GPACDemuxMP4Full();
	bool openData();
	bool updateData();
	bool processSample();
	bool processData();

	std::unique_ptr<ISOProgressiveReader> reader;
};

}
