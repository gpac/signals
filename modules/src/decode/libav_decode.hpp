#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include "../common/libav.hpp"
#include <string>
#include "ffpp.hpp"

struct AVCodecContext;

using namespace Modules;

namespace Decode {

class MODULES_EXPORT LibavDecode : public Module {
public:
	static LibavDecode* create(const PropsDecoder &props);
	~LibavDecode();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	LibavDecode(AVCodecContext *codecCtx);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	struct AVCodecContext *codecCtx;
	ffpp::Frame avFrame;
};

}
