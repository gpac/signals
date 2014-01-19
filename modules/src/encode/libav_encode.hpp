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

class MODULES_EXPORT LibavEncode : public Module {
public:
	static LibavEncode* create(const PropsMuxer &props);
	~LibavEncode();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

private:
	LibavEncode(AVStream *videoStream, AVFrame *avFrame);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	struct AVStream *avStream;
	struct AVFrame *avFrame;
	int frameNum;
};

}
