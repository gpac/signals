#include "jpegturbo_encode.hpp"
#include "lib_utils/log.hpp"
#include "lib_utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}
#include <cassert>


namespace Encode {

class JPEGTurbo {
	public:
		JPEGTurbo() {
			handle = tjInitCompress();
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

JPEGTurboEncode::JPEGTurboEncode(Resolution resolution, int JPEGQuality)
	: jtHandle(new JPEGTurbo), JPEGQuality(JPEGQuality), resolution(resolution) {
	output = addOutput(new OutputDefault);
}

JPEGTurboEncode::~JPEGTurboEncode() {
}

void JPEGTurboEncode::process(std::shared_ptr<const Data> data_) {
	auto data = safe_cast<const PictureRGB24>(data_);
	auto const dataSize = tjBufSize(resolution.width, resolution.height, TJSAMP_420);
	auto out = output->getBuffer(dataSize);
	unsigned long jpegSize;
	unsigned char *buf = (unsigned char*)out->data();
	auto jpegBuf = data->data();
	if (tjCompress2(jtHandle->get(), (unsigned char*)jpegBuf, resolution.width, 0/*pitch*/, resolution.height, TJPF_RGB, &buf, &jpegSize, TJSAMP_420, JPEGQuality, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_encode] error encountered while compressing.");
		return;
	}
	out->resize(jpegSize);
	output->emit(out);
}

}
