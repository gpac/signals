#include "jpegturbo_encode.hpp"
#include "lib_utils/tools.hpp"
extern "C" {
#include <turbojpeg.h>
}
#include <cassert>


namespace Modules {
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

JPEGTurboEncode::JPEGTurboEncode(int JPEGQuality)
	: jtHandle(new JPEGTurbo), JPEGQuality(JPEGQuality) {
	auto input = addInput(new Input<DataPicture>(this));
	input->setMetadata(new MetadataRawVideo);
	output = addOutput(new OutputDefault);
}

JPEGTurboEncode::~JPEGTurboEncode() {
}

void JPEGTurboEncode::process(Data data_) {
	auto data = safe_cast<const PictureRGB24>(data_);
	auto const w = data->getFormat().res.width, h = data->getFormat().res.height;
	auto const dataSize = tjBufSize(w, h, TJSAMP_420);
	auto out = output->getBuffer(dataSize);
	unsigned char *buf = (unsigned char*)out->data();
	auto jpegBuf = data->data();
	unsigned long jpegSize;
	if (tjCompress2(jtHandle->get(), (unsigned char*)jpegBuf, w, 0/*pitch*/, h, TJPF_RGB, &buf, &jpegSize, TJSAMP_420, JPEGQuality, TJFLAG_FASTDCT) < 0) {
		log(Warning, "error encountered while compressing.");
		return;
	}
	out->resize(jpegSize);
	output->emit(out);
}

}
}
