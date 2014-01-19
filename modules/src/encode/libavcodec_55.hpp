#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include "../common/libav.hpp"
#include <string>

struct AVStream;
struct AVFrame;

using namespace Modules;

namespace Encode {

class MODULES_EXPORT Libavcodec_55 : public Module {
public:
	static Libavcodec_55* create(const PropsMuxer &props);
	~Libavcodec_55();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	Libavcodec_55(AVStream *videoStream, AVFrame *avFrame);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	struct AVStream *avStream;
	struct AVFrame *avFrame;
	int frameNum;
};

}
