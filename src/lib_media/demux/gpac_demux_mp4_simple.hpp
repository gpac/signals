#pragma once

#include "lib_modules/core/module.hpp"
#include <cstdint>

typedef struct __tag_isom GF_ISOFile;

using namespace Modules;

namespace Demux {

class ISOFileReader;

class GPACDemuxMP4Simple : public Module {
public:
	GPACDemuxMP4Simple(std::string const& path);
	~GPACDemuxMP4Simple();
	void process(std::shared_ptr<const Data> data) override;

private:

	std::unique_ptr<ISOFileReader> reader;
	OutputDefault* output;
};

}
