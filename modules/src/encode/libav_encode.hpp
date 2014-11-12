#pragma once

#include "internal/core/module.hpp"
#include "../common/libav.hpp"
#include "../common/mm.hpp"

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
		Unknown
	};

	LibavEncode(Type type);
	~LibavEncode();
	void process(std::shared_ptr<Data> data) override;
	void flush() override;

	void sendOutputPinsInfo(); //FIXME: temporary until modules have a manager
	Signal<void(std::shared_ptr<Stream>)> declareStream; //FIXME: temporary until modules have a type 'mux'

private:
	bool processAudio(const PcmData *data);
	bool processVideo(const Picture *data);

	std::string getCodecName() const;

	AVCodecContext *codecCtx;
	std::unique_ptr<PcmFormat> pcmFormat;
	std::unique_ptr<ffpp::Frame> const avFrame;
	int frameNum;
};

}
