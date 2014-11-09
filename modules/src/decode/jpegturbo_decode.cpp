#include "jpegturbo_decode.hpp"
#include "internal/core/clock.hpp"
#include "../common/libav.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}

#include "ffpp.hpp" //TODO: remove once the Props are not based on libav anymore


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
	pins.push_back(uptr(pinFactory->createPin()));
}

JPEGTurboDecode::~JPEGTurboDecode() {
	auto p = dynamic_cast<PropsDecoder*>(pins[0]->getProps());
	if (p) {
		auto ctx = p->getAVCodecContext();
		avcodec_close(ctx);
		av_free(ctx);
	}
}

void JPEGTurboDecode::ensureProps(int width, int height, int pixelFmt) {
	if (!pins[0]->getProps()) {
		auto codec = avcodec_find_decoder_by_name("jpg");
		auto ctx = avcodec_alloc_context3(codec);
		ctx->width = width;
		ctx->height = height;
		ctx->pix_fmt = getAVPF(pixelFmt);
		pins[0]->setProps(new PropsDecoder(ctx));
	}
}

void JPEGTurboDecode::process(std::shared_ptr<Data> data) {
	const int pixelFmt = TJPF_RGB;
	int w, h, jpegSubsamp;
	if (tjDecompressHeader2(jtHandle->get(), data->data(), (unsigned long)data->size(), &w, &h, &jpegSubsamp) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing header.");
		return;
	}
	auto out = pins[0]->getBuffer(w*h*3);
	if (tjDecompress2(jtHandle->get(), data->data(), (unsigned long)data->size(), out->data(), w, 0/*pitch*/, h, pixelFmt, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing frame.");
		return;
	}
	ensureProps(w, h, pixelFmt);
	pins[0]->emit(out);
}

}
