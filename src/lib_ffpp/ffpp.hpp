#pragma once

#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include "libswscale/swscale.h"
}

namespace ffpp {

class Frame {
public:
	Frame() {
		avFrame = av_frame_alloc();
		if(!avFrame)
			throw std::runtime_error("Frame allocation failed");
	}

	~Frame() {
		av_frame_free(&avFrame);
	}

	AVFrame* get() {
		return avFrame;
	}

private:
	AVFrame *avFrame;
};

class Dict {
public:
	Dict() {
		m_AvDict = nullptr;
	}

	~Dict() {
		av_dict_free(&m_AvDict);
	}

	void set(std::string const& name, std::string const& val) {
		av_dict_set(&m_AvDict, name.c_str(), val.c_str(), 0);
	}

	AVDictionaryEntry* get(std::string const name, AVDictionaryEntry* entry = nullptr) {
		return av_dict_get(m_AvDict, name.c_str(), entry, 0);
	}

	AVDictionary** operator&() {
		return &m_AvDict;
	}
private:
	AVDictionary* m_AvDict;
};

class SwResampler {
public:
	SwResampler() {
		m_SwrContext = swr_alloc();
	}

	void setInputLayout(int64_t layout) {
		av_opt_set_int(m_SwrContext, "in_channel_layout", layout, 0);
	}

	void setInputSampleRate(int64_t rate) {
		av_opt_set_int(m_SwrContext, "in_sample_rate", rate, 0);
	}

	void setInputSampleFmt(AVSampleFormat fmt) {
		av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", fmt, 0);
	}

	void setOutputLayout(int64_t layout) {
		av_opt_set_int(m_SwrContext, "out_channel_layout", layout, 0);
	}

	void setOutputSampleRate(int64_t rate) {
		av_opt_set_int(m_SwrContext, "out_sample_rate", rate, 0);
	}

	void setOutputSampleFmt(AVSampleFormat fmt) {
		av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", fmt, 0);
	}

	void init() {
		auto const ret = swr_init(m_SwrContext);
		if(ret < 0)
			throw std::runtime_error("SwResampler: swr_init failed");
	}

	int convert(uint8_t **out, int out_count, const uint8_t **in , int in_count) {
		auto const ret = swr_convert(m_SwrContext, out, out_count, in, in_count);
		if(ret < 0)
			throw std::runtime_error("SwResampler: convert failed");
		return ret;
	}

	int64_t getDelay(int64_t rate) {
		return swr_get_delay(m_SwrContext, rate);
	}

	~SwResampler() {
		swr_free(&m_SwrContext);
	}

private:
	SwrContext* m_SwrContext;
};

}

