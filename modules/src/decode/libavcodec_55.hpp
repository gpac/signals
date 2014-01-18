#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include "../common/libav.hpp"
#include <string>

struct AVCodecContext;
struct AVFrame;

using namespace Modules;

class MODULES_EXPORT Libavcodec_55 : public Module {
public:
	static Libavcodec_55* create(const PropsDecoder &props);
	~Libavcodec_55();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Libavcodec_55(AVCodecContext *codecCtx, AVFrame *frame);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	struct AVCodecContext *codecCtx;
	struct AVFrame *frame;
};
