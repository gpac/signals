#include "libav_demux.hpp"
#include "../transform/restamp.hpp"
#include "../common/libav.hpp"
#include "lib_utils/tools.hpp"
#include "lib_ffpp/ffpp.hpp"
#include <cassert>
#include <fstream>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}


namespace Modules {

namespace {

auto g_InitAv = runAtStartup(&av_register_all);
auto g_InitAvformat = runAtStartup(&avformat_network_init);
auto g_InitAvcodec = runAtStartup(&avcodec_register_all);
auto g_InitAvdevice = runAtStartup(&avdevice_register_all);
auto g_InitAvLog = runAtStartup(&av_log_set_callback, avLog);

const char* webcamFormat() {
#ifdef _WIN32
	return "dshow";
#elif __linux__
	return "v4l2";
#elif __APPLE__
	return "avfoundation";
#else
#error "unknown platform"
#endif
}

bool isRaw(AVCodecContext *codecCtx) {
	return codecCtx->codec_id == AV_CODEC_ID_RAWVIDEO;
}

}

namespace Demux {
void LibavDemux::webcamList() {
	log(Warning, "Webcam list:");
	ffpp::Dict dict;
	buildAVDictionary(typeid(*this).name(), &dict, "-list_devices true", "format");
	avformat_open_input(&m_formatCtx, "video=dummy:audio=dummy", av_find_input_format(webcamFormat()), &dict);
	log(Warning, "Webcam example: webcam:video=\"Integrated Webcam\":audio=\"Microphone (Realtek High Defini\"");
}

bool LibavDemux::webcamOpen(const std::string &options) {
	auto avInputFormat = av_find_input_format(webcamFormat());
	if (avformat_open_input(&m_formatCtx, options.c_str(), avInputFormat, nullptr))
		return false;
	return true;
}

LibavDemux::LibavDemux(const std::string &url) {
	if (!(m_formatCtx = avformat_alloc_context()))
		throw error("Can't allocate format context");

	const std::string device = url.substr(0, url.find(":"));
	if (device == "webcam") {
		if (url == device || !webcamOpen(url.substr(url.find(":") + 1))) {
			webcamList();
			if (m_formatCtx) avformat_close_input(&m_formatCtx);
			throw error("Webcam init failed.");
		}
		restamp = uptr(create<Transform::Restamp>(Transform::Restamp::ClockSystem)); /*some webcams timestamps don't start at 0 (based on UTC)*/
	} else {
		ffpp::Dict dict;
		dict.set("probesize", "100M");
		dict.set("analyzeduration", "100M");
		if (avformat_open_input(&m_formatCtx, url.c_str(), nullptr, &dict)) {
			if (m_formatCtx) avformat_close_input(&m_formatCtx);
			throw error(format("Error when opening input '%s'", url));
		}

		//if you don't call you may miss the first frames
		if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
			avformat_close_input(&m_formatCtx);
			throw error("Couldn't get additional video stream info");
		}

		restamp = uptr(create<Transform::Restamp>(Transform::Restamp::Reset));
	}

	for (unsigned i = 0; i<m_formatCtx->nb_streams; i++) {
		IMetadata *m;
		switch (m_formatCtx->streams[i]->codec->codec_type) {
		case AVMEDIA_TYPE_AUDIO: m = new MetadataPktLibavAudio(m_formatCtx->streams[i]->codec); break;
		case AVMEDIA_TYPE_VIDEO: /*ffpp::isRaw(m_formatCtx->streams[i]->codec) ? m = new MetadataRawVideo :*/
			m = new MetadataPktLibavVideo(m_formatCtx->streams[i]->codec); break;
		default: m = nullptr; break;
		}
		outputs.push_back(addOutput(new OutputDataDefault<DataAVPacket>(m)));
		av_dump_format(m_formatCtx, i, "", 0);
	}
}

LibavDemux::~LibavDemux() {
	avformat_close_input(&m_formatCtx);
}

void LibavDemux::setTime(std::shared_ptr<DataAVPacket> data) {
	auto pkt = data->getPacket();
	auto const base = m_formatCtx->streams[pkt->stream_index]->time_base;
	auto const time = timescaleToClock(pkt->pts * base.num, base.den);
	data->setTime(time);

	restamp->process(data);

	int64_t offset = data->getTime() - time;
	if (offset != 0) {
		/*propagate to AVPacket*/
		data->restamp(offset * base.num, base.den);
	}
}

void LibavDemux::process(Data data) {
	for (;;) {
		if (getNumInputs() && getInput(0)->tryPop(data))
			break;

		auto out = outputs[0]->getBuffer(0);
		AVPacket *pkt = out->getPacket();
		int status = av_read_frame(m_formatCtx, pkt);
		if (status < 0) {
			if (status == (int)AVERROR_EOF || (m_formatCtx->pb && m_formatCtx->pb->eof_reached)) {
			} else if (m_formatCtx->pb && m_formatCtx->pb->error) {
				log(Warning, "Stream contains an irrecoverable error - leaving");
			}
			return;
		}

		setTime(out);

		outputs[pkt->stream_index]->emit(out);
	}

	log(Info, "Exit from an external event.");
}

}
}
