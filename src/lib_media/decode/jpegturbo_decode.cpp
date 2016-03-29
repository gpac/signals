#include "jpegturbo_decode.hpp"
#include "../common/libav.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}

#include "lib_ffpp/ffpp.hpp"


namespace Modules {
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
	case TJPF_RGB: return AV_PIX_FMT_RGB24;
	default: throw std::runtime_error("[JPEGTurboDecode] Unsupported pixel format conversion. Failed.");
	}
	return AV_PIX_FMT_NONE;
}

JPEGTurboDecode::JPEGTurboDecode()
	: jtHandle(new JPEGTurbo) {
	auto input = addInput(new Input<DataBase>(this));
	input->setMetadata(new MetadataPktVideo);
	output = addOutput(new OutputPicture(new MetadataRawVideo));
}

JPEGTurboDecode::~JPEGTurboDecode() {
}

void JPEGTurboDecode::ensureMetadata(int width, int height, int pixelFmt) {
	if (!output->getMetadata()) {
		auto p = safe_cast<const MetadataRawVideo>(output->getMetadata());
		//TODO: add resolution and pixel format to MetadataRawVideo
		//ctx->width = width;
		//ctx->height = height;
		//ctx->pix_fmt = getAVPF(pixelFmt);
		//output->setMetadata(new MetadataRawVideo(ctx));
	}
}

void JPEGTurboDecode::process(Data data_) {
	auto data = safe_cast<const DataBase>(data_);
	const int pixelFmt = TJPF_RGB;
	int w, h, jpegSubsamp;
	auto jpegBuf = data->data();
	if (tjDecompressHeader2(jtHandle->get(), (unsigned char*)jpegBuf, (unsigned long)data->size(), &w, &h, &jpegSubsamp) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing header.");
		return;
	}
	auto out = DataPicture::create(output, Resolution(w, h), RGB24);
	if (tjDecompress2(jtHandle->get(), (unsigned char*)jpegBuf, (unsigned long)data->size(), out->data(), w, 0/*pitch*/, h, pixelFmt, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing frame.");
		return;
	}
	ensureMetadata(w, h, pixelFmt);
	output->emit(out);
}

}
}
