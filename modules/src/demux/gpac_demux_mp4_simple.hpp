#pragma once

#include "internal/core/module.hpp"
#include <cstdint>

typedef struct __tag_isom GF_ISOFile;

using namespace Modules;

namespace Demux {

class ISOFileReader;

class GPACDemuxMP4Simple : public Module {
public:
	static GPACDemuxMP4Simple* create(std::string const& path);
	~GPACDemuxMP4Simple();
	void process(std::shared_ptr<const Data> data) override;

private:
	GPACDemuxMP4Simple(GF_ISOFile *movie);

	std::unique_ptr<ISOFileReader> reader;
	PinDefault* output;
};

}
