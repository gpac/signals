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

	void sendOutputPinsInfo(); //FIXME: temporary until modules have a manager
	Signal<void(std::shared_ptr<Stream>)> declareStream; //FIXME: temporary until modules have a type 'mux'

private:
	bool processAudio(const DataAVFrame *data);
	bool processVideo(const DataAVFrame *data);

	std::string getCodecName() const;

	AVCodecContext *codecCtx;
	int frameNum;
};

}
