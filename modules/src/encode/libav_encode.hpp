#pragma once

#include "../../internal/config.hpp"
#include "internal/module.hpp"
#include "internal/param.hpp"
#include "../common/libav.hpp"
#include "../common/mm.hpp"
#include <string>
#include "ffpp.hpp"

struct AVStream;

using namespace Modules;

namespace Encode {

class MODULES_EXPORT LibavEncode : public Module {
public:
	enum Type {
		Video,
		Audio,
		unknown
	};

	static LibavEncode* create(Type type);
	~LibavEncode();
	bool process(std::shared_ptr<Data> data);
	bool handles(const std::string &url);
	static bool canHandle(const std::string &url);

	void sendOutputPinsInfo(); //FIXME: temporary until modules have a manager
	Signal<void(std::shared_ptr<StreamVideo>)> declareStream; //FIXME: temporary until modules have a type 'mux'

private:
	LibavEncode(Type type);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	AVCodecContext *codecCtx;
	ffpp::Frame avFrame;
	int frameNum;

};

}
