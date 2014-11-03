#include "mpeg_dash.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
#include "../out/file.hpp"
#include "../common/libav.hpp" //FIXME: for DataAVPacket

#include "ffpp.hpp" //FIXME: remove once not based on libav anymore

namespace Modules {
namespace Stream {

#define DASH_DUR_IN_MS 10000

MPEG_DASH::MPEG_DASH(Type type)
: type(type), workingThread(&MPEG_DASH::DASHThread, this) {
}

MPEG_DASH::~MPEG_DASH() {
	//FIXME: the right thing to do would be to wait until it is empty
	audioDataQueue.clear();
	videoDataQueue.clear();
	workingThread.join();
}

//FIXME: we would post/defer/schedule the whole module... but here we are already in our own thread
void MPEG_DASH::DASHThread() {
	uint64_t n = 0;
	auto startTime = std::chrono::steady_clock::now();
	for (;;) {
		auto a = audioDataQueue.pop();
		auto v = videoDataQueue.pop();
		if (!a || !v)
			break;

		GenerateMPD(n, a, v);

		if (type == Live) {
			auto now = std::chrono::steady_clock::now();
			auto next = startTime + std::chrono::milliseconds(DASH_DUR_IN_MS * n);
			if (next > now) {
				auto dur = next - now;
				std::this_thread::sleep_for(dur);
			} else {
				Log::msg(Log::Warning, "[MPEG_DASH] Next MPD update (%s) is in the past. Are we running too slow?", n);
			}
		}

		n++;
	}
}

void MPEG_DASH::GenerateMPD(uint64_t segNum, std::shared_ptr<Data> /*audio*/, std::shared_ptr<Data> /*video*/) {
#if 0
	//Print the segments to the appropriate threads
	std::stringstream ssa, ssv;

	const std::string audioFn = "audio.m4s";
	const std::string videoFn = "video.m4s";

	//serialize the MPD (use the GPAC code?)
	auto audioSeg = uptr(Out::File::create(audioFn));
	auto videoSeg = uptr(Out::File::create(videoFn));
#else
	//printf("%d\n", segNum);
#endif
}

void MPEG_DASH::process(std::shared_ptr<Data> data) {
#if 0
	/* TODO:
	 * 1) no test on timestamps
	 * 2) no time to provoke the MPD generation on time
	 */
	auto inputData = dynamic_cast<DataAVPacket*>(data.get());
	if (!inputData) {
		Log::msg(Log::Warning, "[MPEG_DASH] Invalid data type.");
		return;
	}

	AVPacket *pkt = inputData->getPacket();
	switch (pkt->stream_index) {
	//FIXME: arbitrary
	case 0:
		audioDataQueue.push(data);
		break;
	case 1:
		videoDataQueue.push(data);
		break;
	default:
		Log::msg(Log::Warning, "[MPEG_DASH] undeclared data. Discarding.");
		return;
	}
#else
	assert(0);
#endif
}

void MPEG_DASH::processAudio(std::shared_ptr<Data> data) {
	audioDataQueue.push(data);
}

void MPEG_DASH::processVideo(std::shared_ptr<Data> data) {
	videoDataQueue.push(data);
}

}
}
