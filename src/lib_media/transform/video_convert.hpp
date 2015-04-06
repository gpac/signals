#pragma once

#include "../common/picture.hpp"
#include "lib_modules/core/module.hpp"
#include "lib_ffpp/ffpp.hpp"

namespace Modules {
namespace Transform {

class VideoConvert : public Module {
public:
	VideoConvert(const PictureFormat &dstFormat);
	~VideoConvert();
	void process(std::shared_ptr<const Data> data) override;

private:
	void reconfigure(const PictureFormat &format);

	SwsContext *m_SwContext;
	PictureFormat srcFormat, dstFormat;
	PacketAllocator<PictureYUV420> picAlloc;
	PacketAllocator<PictureRGB24> rawAlloc;
	PinDefault* output;
};

}
}
