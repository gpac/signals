#pragma once
#include "ffpp.hpp"

// TODO move this to its own module, when we have media types
class AudioConverter {
public:
	AudioConverter(AVCodecContext const& src, Pin& pin)
		: m_Pin(pin) {
		m_Swr.setInputSampleFmt(src.sample_fmt);
		m_Swr.setInputLayout(src.channel_layout);
		m_Swr.setInputSampleRate(src.sample_rate);

		m_Swr.setOutputSampleFmt(DST_FMT);
		m_Swr.setOutputLayout(DST_LAYOUT);
		m_Swr.setOutputSampleRate(DST_FREQ);

		m_Swr.init();
	}

	std::shared_ptr<Data> convert(AVCodecContext* codecCtx, AVFrame* avFrame) {
		const int bufferSize = av_samples_get_buffer_size(nullptr, codecCtx->channels, avFrame->nb_samples, codecCtx->sample_fmt, 0);

		auto const srcNumSamples = avFrame->nb_samples;
		auto const dstNumSamples = divUp(srcNumSamples * DST_FREQ, codecCtx->sample_rate);

		auto out = std::dynamic_pointer_cast<PcmData>(m_Pin.getBuffer(bufferSize * 10));

		uint8_t* pDst = out->data();
		auto const numSamples = m_Swr.convert(&pDst, dstNumSamples, (const uint8_t**)avFrame->data, srcNumSamples);

		auto const dstChannels = av_get_channel_layout_nb_channels(DST_LAYOUT);
		auto const sampleSize = av_samples_get_buffer_size(nullptr, dstChannels, numSamples, DST_FMT, 1);

		out->resize(sampleSize);

		return out;
	}

private:
	static const auto DST_FREQ = AUDIO_SAMPLERATE;
	static const uint64_t DST_LAYOUT = AV_CH_LAYOUT_STEREO;
	static const auto DST_FMT = AV_SAMPLE_FMT_S16;
	ffpp::SwResampler m_Swr;
	Pin& m_Pin;
};

// TODO move this to its own module, when we have media types
class VideoConverter {
public:
	VideoConverter(AVCodecContext const& codecCtx, Pin& pin)
		: m_Pin(pin) {
		m_SwContext = sws_getContext(
		                  codecCtx.width, codecCtx.height, codecCtx.pix_fmt,
		                  DST_WIDTH, DST_HEIGHT, DST_FMT,
		                  2, nullptr, nullptr, nullptr);
	}

	std::shared_ptr<Data> convert(AVCodecContext* codecCtx, AVFrame* avFrame) {
		const auto srcHeight = codecCtx->height;

		const int dstFrameSize = (DST_WIDTH * DST_HEIGHT * 3) / 2;
		auto out(m_Pin.getBuffer(dstFrameSize));

		uint8_t* pDst[3] = {
			out->data(),
			out->data() + DST_WIDTH * DST_HEIGHT,
			out->data() + DST_WIDTH * DST_HEIGHT * 5/4,
		};

		int dstStride[3] = {DST_WIDTH, DST_WIDTH/2, DST_WIDTH/2};

		sws_scale(m_SwContext,
		          avFrame->data, avFrame->linesize, 0, srcHeight,
		          pDst, dstStride);

		return out;
	}

	~VideoConverter() {
		sws_freeContext(m_SwContext);
	}

private:
	static const auto DST_WIDTH = VIDEO_WIDTH;
	static const auto DST_HEIGHT = VIDEO_HEIGHT;
	static const auto DST_FMT = PIX_FMT_YUV420P;
	SwsContext* m_SwContext;
	Pin& m_Pin;
};
