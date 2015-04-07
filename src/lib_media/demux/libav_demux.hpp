#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Demux {

class LibavDemux : public Module {
public:
	LibavDemux(const std::string &url, const std::string &options = "");
	~LibavDemux();
	void process(std::shared_ptr<const Data> data) override;

private:
	struct AVFormatContext *m_formatCtx;
	std::vector<PinDataDefault<DataAVPacket>*> outputs;
};

}
