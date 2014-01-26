#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include <cstdint>
#include <string>

using namespace Modules;

namespace Demux {

class ISOProgressiveReader;

class MODULES_EXPORT GPACDemuxMP4Full : public Module {
public:
	static GPACDemuxMP4Full* create();
	~GPACDemuxMP4Full();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url); //FIXME: useless for memory-based module...
	static bool canHandle(const std::string &url); //FIXME: useless for memory-based module...

private:
	GPACDemuxMP4Full();
	bool openData();
	bool updateData();
	bool processSample();
	bool processData();

	std::unique_ptr<ISOProgressiveReader> reader;
};

}
