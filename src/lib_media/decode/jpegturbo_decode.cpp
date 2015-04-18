#include "jpegturbo_decode.hpp"
#include "../common/libav.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}

#include "lib_ffpp/ffpp.hpp" //TODO: remove once the Metadata are not based on libav anymore


namespace Decode {

class JPEGTurbo {
public:
	JPEGTurbo() {
		handle = tjInitDecompress();
	}
	~JPEGTurbo() {
		tjDestroy(handle);
	}
	tjhandle get() {
		return handle;
	}

private:
	tjhandle handle;
};

AVPixelFormat getAVPF(int JPEGTurboPixelFmt) {
	switch (JPEGTurboPixelFmt) {
	case TJPF_RGB:
		return AV_PIX_FMT_RGB24;
	default:
		throw std::runtime_error("[JPEGTurboDecode] Unsupported pixel format conversion. Failed.");
	}
	return AV_PIX_FMT_NONE;
}

JPEGTurboDecode::JPEGTurboDecode()
	: jtHandle(new JPEGTurbo) {
	output = addOutput(new OutputDefault);
}

JPEGTurboDecode::~JPEGTurboDecode() {
	auto p = safe_cast<const MetadataPktLibav>(output->getMetadata());
	if (p) {
		auto ctx = p->getAVCodecContext();
		avcodec_close(ctx);
		av_free(ctx);
	}
}

void JPEGTurboDecode::ensureMetadata(int width, int height, int pixelFmt) {
	if (!output->getMetadata()) {
		auto codec = avcodec_find_decoder_by_name("jpg");
		auto ctx = avcodec_alloc_context3(codec);
		ctx->width = width;
		ctx->height = height;
		ctx->pix_fmt = getAVPF(pixelFmt);
		output->setMetadata(new MetadataPktLibavVideo(ctx));
	}
}

void JPEGTurboDecode::process(Data data_) {
	auto data = safe_cast<const RawData>(data_);
	const int pixelFmt = TJPF_RGB;
	int w, h, jpegSubsamp;
	auto jpegBuf = data->data();
	if (tjDecompressHeader2(jtHandle->get(), (unsigned char*)jpegBuf, (unsigned long)data->size(), &w, &h, &jpegSubsamp) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing header.");
		return;
	}
	auto out = output->getBuffer(w*h*3);
	if (tjDecompress2(jtHandle->get(), (unsigned char*)jpegBuf, (unsigned long)data->size(), out->data(), w, 0/*pitch*/, h, pixelFmt, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing frame.");
		return;
	}
	ensureMetadata(w, h, pixelFmt);
	output->emit(out);
}

}
