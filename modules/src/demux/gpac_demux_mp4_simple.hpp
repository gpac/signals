#pragma once

#include "internal/module.hpp"
#include <cstdint>
#include <string>

typedef struct __tag_isom GF_ISOFile;

using namespace Modules;

namespace Demux {

class ISOFileReader;

class GPACDemuxMP4Simple : public Module {
public:
	static GPACDemuxMP4Simple* create(std::string const& path);
	~GPACDemuxMP4Simple();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	GPACDemuxMP4Simple(GF_ISOFile *movie);

	std::unique_ptr<ISOFileReader> reader;
};

}
