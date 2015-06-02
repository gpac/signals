#include "gpac_mux_m2ts.hpp"
#include "../common/libav.hpp"
#include <cassert>

extern "C" {
#include <libavformat/avformat.h>
}

namespace Modules {

namespace Mux {

GPACMuxMPEG2TS::GPACMuxMPEG2TS() {
}

GPACMuxMPEG2TS::~GPACMuxMPEG2TS() {
}

void GPACMuxMPEG2TS::declareStream(Data data) {
	auto const metadata_ = data->getMetadata();
#if 0 //TODO: a similar function is needed to declare inputs dynamically
	if (auto metadata = std::dynamic_pointer_cast<const MetadataPktLibavVideo>(metadata_)) {
		AVStream *avStream = avformat_new_stream(m_formatCtx, metadata->getAVCodecContext()->codec);
		if (!avStream) {
			Log::msg(Log::Warning, "[GPACMuxMPEG2TS] could not create the stream, disable output.");
			throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed.");
		}

		m_formatCtx->streams[0]->codec->time_base = metadata->getAVCodecContext()->time_base; //FIXME: [0]: not a mux yet...
		m_formatCtx->streams[0]->codec->width = metadata->getAVCodecContext()->width;
		m_formatCtx->streams[0]->codec->height = metadata->getAVCodecContext()->height;
		if (m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
			m_formatCtx->streams[0]->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavVideo(metadata->getAVCodecContext()));
	} else if (auto metadata2 = std::dynamic_pointer_cast<const MetadataPktLibavAudio>(metadata_)) {
		AVStream *avStream = avformat_new_stream(m_formatCtx, metadata2->getAVCodecContext()->codec);
		if (!avStream) {
			Log::msg(Log::Warning, "[GPACMuxMPEG2TS] could not create the stream, disable output.");
			throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed.");
		}

		m_formatCtx->streams[0]->codec->sample_rate = metadata2->getAVCodecContext()->sample_rate;
		auto input = addInput(new Input<DataAVPacket>(this));
		input->setMetadata(new MetadataPktLibavAudio(metadata2->getAVCodecContext()));
	} else {
		throw std::runtime_error("[GPACMuxMPEG2TS] Stream creation failed: unknown type.");
	}
#endif
}

void GPACMuxMPEG2TS::process() {
	//FIXME: reimplement with multiple inputs
	Data data = inputs[0]->pop();
	if (inputs[0]->updateMetadata(data))
		declareStream(data);
	auto encoderData = safe_cast<const DataAVPacket>(data);
	/*auto pkt =*/ encoderData->getPacket();

	/* write the compressed frame to the output */
	//TODO
}

}
}
