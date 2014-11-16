#include "jpegturbo_encode.hpp"
#include "internal/core/clock.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
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

JPEGTurboEncode::JPEGTurboEncode(int width, int height, int JPEGQuality)
	: jtHandle(new JPEGTurbo), JPEGQuality(JPEGQuality), width(width), height(height) {
	PinDefaultFactory pinFactory;
	pins.push_back(uptr(pinFactory.createPin()));
}

JPEGTurboEncode::~JPEGTurboEncode() {
}

void JPEGTurboEncode::process(std::shared_ptr<const Data> data) {
	auto out = pins[0]->getBuffer(tjBufSize(width, height, TJSAMP_420));
	unsigned long jpegSize;
	unsigned char *buf = (unsigned char*)out->data();
	unsigned char *jpegBuf = const_cast<Data*>(data.get())->data();
	if (tjCompress2(jtHandle->get(), jpegBuf, width, 0/*pitch*/, height, TJPF_RGB, &buf, &jpegSize, TJSAMP_420, JPEGQuality, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_encode] error encountered while compressing.");
		return;
	}
	out->resize(jpegSize);
	pins[0]->emit(out);
}

}
