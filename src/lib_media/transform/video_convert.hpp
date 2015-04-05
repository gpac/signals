#pragma once

#include "lib_modules/core/module.hpp"
#include "lib_ffpp/ffpp.hpp"

namespace Modules {
namespace Transform {

class VideoConvert : public Module {
	public:
		VideoConvert(Resolution dstRes, AVPixelFormat dstFormat);
		~VideoConvert();
		void process(std::shared_ptr<const Data> data) override;

	private:
		void reconfigure(const Resolution &srcRes, const AVPixelFormat &srcFormat);

		SwsContext *m_SwContext;
		Resolution m_srcRes, dstRes;
		AVPixelFormat dstFormat;
		PacketAllocator<Picture> picAlloc;
		PacketAllocator<RawData> rawAlloc;
		PinDefault* output;
};

}
}
