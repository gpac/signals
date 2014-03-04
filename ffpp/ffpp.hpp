#pragma once

#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

namespace ffpp {
struct Frame {
	Frame() {
		avFrame = avcodec_alloc_frame();
		if(!avFrame)
			throw std::runtime_error("Frame allocation failed");
	}

	~Frame() {
		avcodec_free_frame(&avFrame);
	}

	AVFrame* get() {
		return avFrame;
	}

private:
	AVFrame *avFrame;
};

struct Dict {
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

	AVDictionaryEntry* get(std::string const name, AVDictionaryEntry* entry) {
		return av_dict_get(m_AvDict, name.c_str(), entry, 0);
	}

	AVDictionary** operator&() {
		return &m_AvDict;
	}
private:
	AVDictionary* m_AvDict;
};

}

