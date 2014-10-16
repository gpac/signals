#include "jpegturbo_decode.hpp"
#include "internal/core/clock.hpp"
#include "../utils/log.hpp"
#include "../utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}


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

JPEGTurboDecode::JPEGTurboDecode()
	: jtHandle(new JPEGTurbo) {
	signals.push_back(uptr(pinFactory->createPin()));
}

JPEGTurboDecode::~JPEGTurboDecode() {
}

void JPEGTurboDecode::process(std::shared_ptr<Data> data) {
	int w, h, jpegSubsamp;
	if (tjDecompressHeader2(jtHandle->get(), data->data(), (unsigned long)data->size(), &w, &h, &jpegSubsamp) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing header.");
		return;
	}
	auto out = signals[0]->getBuffer(w*h*3);
	if (tjDecompress2(jtHandle->get(), data->data(), (unsigned long)data->size(), out->data(), w, 0/*pitch*/, h, TJPF_RGB, TJFLAG_FASTDCT) < 0) {
		Log::msg(Log::Warning, "[jpegturbo_decode] error encountered while decompressing frame.");
		return;
	}
	signals[0]->emit(out);
}

}
