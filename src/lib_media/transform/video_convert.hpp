#pragma once

#include "../common/picture.hpp"
#include "lib_modules/core/module.hpp"

struct SwsContext;

namespace Modules {
namespace Transform {

class VideoConvert : public ModuleS {
public:
	VideoConvert(const PictureFormat &dstFormat);
	~VideoConvert();
	void process(Data data) override;

private:
	void reconfigure(const PictureFormat &format);

	SwsContext *m_SwContext;
	PictureFormat srcFormat, dstFormat;
	OutputPicture* output;
};

}
}
