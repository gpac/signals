#pragma once

#include "internal/core/module.hpp"
#include "ffpp.hpp"

namespace Modules {
namespace Transform {

class VideoConvert : public Module {
	public:
		VideoConvert(Resolution srcRes, AVPixelFormat srcFormat, Resolution dstRes, AVPixelFormat dstFormat);
		~VideoConvert();
		void process(std::shared_ptr<Data> data) override;

	private:
		SwsContext *m_SwContext;
		Resolution srcRes, dstRes;
		AVPixelFormat dstFormat;
};

}
}
