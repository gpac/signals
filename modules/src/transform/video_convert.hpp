#pragma once

#include "internal/core/module.hpp"
#include "ffpp.hpp"
#include <string>

namespace Modules {
namespace Transform {

class VideoConvert : public Module {
	public:
		VideoConvert(int srcW, int srcH, AVPixelFormat srcFormat, int dstW, int dstH, AVPixelFormat dstFormat);
		~VideoConvert();
		void process(std::shared_ptr<Data> data);

	private:
		SwsContext *m_SwContext;
		int srcW;
		int srcH;
		int dstW;
		int dstH;
		AVPixelFormat dstFormat;
};

}
}
