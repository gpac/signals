#pragma once

#include "internal/core/module.hpp"
#include "../common/libav.hpp"
#include "../common/mm.hpp"
#include <string>

struct AVStream;

namespace ffpp {
struct Frame;
}

using namespace Modules;

namespace Encode {

class LibavEncode : public Module {
public:
	enum Type {
		Video,
		Audio,
		unknown
	};

	static LibavEncode* create(Type type);
	~LibavEncode();
	void process(std::shared_ptr<Data> data);

	void sendOutputPinsInfo(); //FIXME: temporary until modules have a manager
	Signal<void(std::shared_ptr<Stream>)> declareStream; //FIXME: temporary until modules have a type 'mux'

private:
	LibavEncode(Type type);
	bool processAudio(std::shared_ptr<Data> data);
	bool processVideo(std::shared_ptr<Data> data);

	std::string getCodecName() const;

	AVCodecContext *codecCtx;
	std::unique_ptr<ffpp::Frame> const avFrame;
	int frameNum;
};

}
