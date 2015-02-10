#pragma once

#include "lib_modules/internal/core/module.hpp"
#include "lib_ffpp/ffpp.hpp"

namespace Modules {
namespace Transform {

class VideoConvert : public Module {
	public:
		VideoConvert(Resolution srcRes, AVPixelFormat srcFormat, Resolution dstRes, AVPixelFormat dstFormat);
		~VideoConvert();
		void process(std::shared_ptr<const Data> data) override;

	private:
		SwsContext *m_SwContext;
		Resolution srcRes, dstRes;
		AVPixelFormat dstFormat;
		PacketAllocator<Picture> picAlloc;
		PacketAllocator<RawData> rawAlloc;
};

}
}
