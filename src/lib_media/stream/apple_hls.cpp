#include "apple_hls.hpp"
#include "lib_modules/core/clock.hpp"
#include "lib_utils/log.hpp"
#include "../common/libav.hpp"


namespace Modules {
namespace Stream {

Apple_HLS::Apple_HLS(std::string const url, Type type, std::string const httpPrefix /* = "" */, uint64_t segDurationInMs /* = 10 */)
: url(url), type(type), segDurationInMs(segDurationInMs), index(0) {
	addInput(new Input<DataAVPacket>(this));

	if (size_t pos = url.rfind("\\") != std::string::npos) {
		char *str = new char[url.length() - pos];
		url.copy(str, url.length() - pos, pos + 1);
		name = std::string(str);
	} else {
		name = url;
	}
	//manifestFile = new Manifest(url, httpPrefix, segDurationInMs);
	currentFile    = fopen(url.c_str(), "w");
}

void Apple_HLS::endOfStream() {
	if (workingThread.joinable()) {
		for (size_t i = 0; i < inputs.size(); ++i)
			inputs[i]->push(nullptr);
		workingThread.join();
	}
}

Apple_HLS::~Apple_HLS() {
	fclose(currentSegment);
	endOfStream();
}

void Apple_HLS::HLSThread() {
	for (;;) {
		std::vector<Data> data;
		data.resize(getNumInputs() - 1);
		for (size_t i = 0; i < getNumInputs() - 1; ++i) {
			uint64_t durationInMs = 0;
			while (1) {
				data[i] = inputs[i]->pop(); //TODO: make pop() multiple times until you have enough data (i.e. segment_duration, 10s)
				if (!data[i]) {
					return;
				}

				durationInMs += clockToTimescale(data[i]->getDuration(), 1000);
				/* if (durationInMs > segDurationInMs) {
					manifestFile.update(index);
					++index;
					fwrite(currentSegment, data[i]->data);
					fclose(currentSegment);
					currentSegment = fopen(url, "w");					
					break;
				} */
			}

		}

#if 0 // 		
	if (type == Live) {
			auto dur = std::chrono::milliseconds(nextInMs);
			Log::msg(Log::Info, "[Apple_HLS] Going to sleep for %s ms.", std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
			std::this_thread::sleep_for(dur);
		}
#endif
	}
}

void Apple_HLS::process() {
	numDataQueueNotify = (int)getNumInputs() - 1; //FIXME: connection/disconnection cannot occur dynamically. Lock inputs?
	if (!workingThread.joinable())
		workingThread = std::thread(&Apple_HLS::HLSThread, this);
}

void Apple_HLS::flush() {
	numDataQueueNotify--;
	if ((type == Live) && (numDataQueueNotify == 0))
		endOfStream();
}

}
}

