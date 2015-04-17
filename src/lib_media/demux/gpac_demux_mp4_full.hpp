#pragma once

#include "lib_modules/core/module.hpp"
#include <cstdint>

using namespace Modules;

namespace Demux {

class ISOProgressiveReader;

class GPACDemuxMP4Full : public ModuleS {
public:
	GPACDemuxMP4Full();
	~GPACDemuxMP4Full();
	void process(std::shared_ptr<const Data> data) override;

private:
	bool openData();
	bool updateData();
	bool processSample();
	bool processData();

	std::unique_ptr<ISOProgressiveReader> reader;
	OutputDefault* output;
};

}
