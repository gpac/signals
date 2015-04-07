#pragma once

#include "lib_modules/core/module.hpp"
#include "../common/libav.hpp"

struct AVFormatContext;

using namespace Modules;

namespace Demux {

class LibavDemux : public Module {
public:
	/**
	 @param url may be a file, a remote URL, or a webcam (set "webcam" to list the available devices)
	 */
	LibavDemux(const std::string &url);
	~LibavDemux();
	void process(std::shared_ptr<const Data> data) override;

private:
	void webcamList();
	bool webcamOpen(const std::string &options);

	struct AVFormatContext *m_formatCtx;
	std::vector<PinDataDefault<DataAVPacket>*> outputs;
};

}
