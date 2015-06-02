#include "apple_hls.hpp"
#include "lib_modules/core/clock.hpp"
#include "lib_utils/log.hpp"
#include "../common/libav.hpp"


namespace Modules {
namespace Stream {

Apple_HLS::Apple_HLS(Type type, uint64_t segDurationInMs)
: type(type), segDurationInMs(segDurationInMs) {
	addInput(new Input<DataAVPacket>(this));
}

void Apple_HLS::endOfStream() {
	if (workingThread.joinable()) {
		for (size_t i = 0; i < inputs.size(); ++i)
			inputs[i]->push(nullptr);
		workingThread.join();
	}
}

Apple_HLS::~Apple_HLS() {
	endOfStream();
}

void Apple_HLS::HLSThread() {
	for (;;) {
		std::vector<Data> data;
		data.resize(getNumInputs() - 1);
		for (size_t i = 0; i < getNumInputs() - 1; ++i) {
			data[i] = inputs[i]->pop(); //TODO: make pop() multiple times until you have enough data (i.e. segment_duration, 10s)
			if (!data[i]) {
				return;
			}

			//TODO: do sth with data[i]
		}

		u32 nextInMs = GenerateM3U8();

		if (type == Live) {
			auto dur = std::chrono::milliseconds(nextInMs);
			Log::msg(Log::Info, "[Apple_HLS] Going to sleep for %s ms.", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
			std::this_thread::sleep_for(dur);
		}
	}
}

void Apple_HLS::process() {
	numDataQueueNotify = (int)getNumInputs() - 1; //FIXME: connection/disconnection cannot occur dynamically. Lock inputs?
	if (!workingThread.joinable())
		workingThread = std::thread(&Apple_HLS::HLSThread, this);
}

u32 Apple_HLS::GenerateM3U8() {
	return 0;
}

void Apple_HLS::flush() {
	numDataQueueNotify--;
	if ((type == Live) && (numDataQueueNotify == 0))
		endOfStream();
}

}
}
