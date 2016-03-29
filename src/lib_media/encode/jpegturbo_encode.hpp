#pragma once

#include "../common/picture.hpp"
#include "lib_modules/core/module.hpp"

#define JPEG_DEFAULT_QUALITY 70

namespace Modules {
namespace Encode {

class JPEGTurbo;

class JPEGTurboEncode : public ModuleS {
	public:
		JPEGTurboEncode(int JPEGQuality = JPEG_DEFAULT_QUALITY);
		~JPEGTurboEncode();
		void process(Data data) override;

	private:
		OutputDefault* output;
		std::unique_ptr<JPEGTurbo> const jtHandle;
		int JPEGQuality;
};

}
}
