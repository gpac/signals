#pragma once

#include "picture.hpp"
#include "lib_modules/core/output.hpp"
#include "lib_modules/core/metadata.hpp"
#include <cstdarg>
#include <memory>

struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct AVDictionary;
#ifdef _MSC_VER
enum AVPixelFormat;
#undef PixelFormat
#else
extern "C" {
#include <libavcodec/avcodec.h>
#undef PixelFormat
}
#endif

namespace Modules {

class MetadataPktLibav : public IMetadata {
	public:
		//Doesn't take codecCtx ownership
		MetadataPktLibav(AVCodecContext *codecCtx);
		virtual ~MetadataPktLibav() {}
		StreamType getStreamType() const override;
		AVCodecContext* getAVCodecContext() const;

	protected:
		AVCodecContext *codecCtx;
};

class MetadataPktLibavVideo : public MetadataPktLibav {
	public:
		MetadataPktLibavVideo(AVCodecContext *codecCtx) : MetadataPktLibav(codecCtx) {}
		PixelFormat getPixelFormat() const;
		Resolution getResolution() const;
		uint32_t getTimeScale() const;
		void getExtradata(const uint8_t *&extradata, size_t &extradataSize) const;
};

class MetadataPktLibavAudio : public MetadataPktLibav {
	public:
		MetadataPktLibavAudio(AVCodecContext *codecCtx) : MetadataPktLibav(codecCtx) {}
		std::string getCodecName() const;
		uint32_t getNumChannels() const;
		uint32_t getSampleRate() const;
		uint8_t getBitsPerSample() const;
		uint32_t getFrameSize() const;
		void getExtradata(const uint8_t *&extradata, size_t &extradataSize) const;
};

class DataAVPacket : public DataBase {
	public:
		DataAVPacket(size_t size = 0);
		~DataAVPacket();
		uint8_t* data() override;
		uint8_t const* data() const override;
		uint64_t size() const override;
		void resize(size_t size) override;

		AVPacket* getPacket() const;
		void restamp(int64_t offsetIn180k, uint64_t pktTimescale) const;

	private:
		std::unique_ptr<AVPacket> const pkt;
};

class PcmFormat;
class DataPcm;
void libavAudioCtxConvertLibav(const PcmFormat *cfg, int &sampleRate, int &format, int &numChannels, uint64_t &layout);
void libavAudioCtxConvert(const PcmFormat *cfg, AVCodecContext *codecCtx);
void libavFrameDataConvert(const DataPcm *data, AVFrame *frame);
void libavFrame2pcmConvert(const AVFrame *frame, PcmFormat *cfg);


void pixelFormat2libavPixFmt(const enum PixelFormat format, enum AVPixelFormat &avPixfmt);
enum PixelFormat libavPixFmt2PixelFormat(const enum AVPixelFormat &avPixfmt);

void buildAVDictionary(const std::string &moduleName, AVDictionary **dict, const char *options, const char *type);

void avLog(void *avcl, int level, const char *fmt, va_list vl);

}
